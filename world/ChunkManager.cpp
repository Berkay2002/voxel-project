#include "ChunkManager.h"
#include "SpaghettiCaveCarver.h"
#include "core/graphics/Shader.h"
#include "core/scene/Camera.h"
#include "core/scene/Frustum.h"
#include "core/Logger.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace Voxel {

ChunkManager::ChunkManager() {
    // TerrainConfig defaults are now set from WorldConfig.h
    // Just use default config - all values come from centralized WorldConfig.h
    m_TerrainGenerator.SetConfig(TerrainConfig{});
    
    // Add spaghetti cave carver for Minecraft-style winding tunnels
    m_TerrainGenerator.AddCaveCarver(std::make_unique<SpaghettiCaveCarver>());
    
    LOG_INFO("ChunkManager initialized with load radius " + std::to_string(m_LoadRadius) +
             ", thread pool size: " + std::to_string(m_ThreadPool.get_thread_count()) +
             ", caves enabled with " + std::to_string(m_TerrainGenerator.GetCaveCarverCount()) + " carver(s)");
}

ChunkManager::~ChunkManager() {
    // Wait for all pending tasks to complete before destroying
    m_ThreadPool.wait();
    LOG_INFO("ChunkManager destroyed, cleaned up " + std::to_string(m_Chunks.size()) + " chunks");
}

ChunkCoord ChunkManager::WorldToChunkCoord(const glm::vec3& worldPos) const {
    // Floor division to get chunk coordinates
    int chunkX = static_cast<int>(std::floor(worldPos.x / CHUNK_WIDTH));
    int chunkZ = static_cast<int>(std::floor(worldPos.z / CHUNK_DEPTH));
    return {chunkX, chunkZ};
}

void ChunkManager::Update(const glm::vec3& playerPos) {
    ChunkCoord currentCenter = WorldToChunkCoord(playerPos);

    // Load chunks within load radius (async)
    for (int dx = -m_LoadRadius; dx <= m_LoadRadius; ++dx) {
        for (int dz = -m_LoadRadius; dz <= m_LoadRadius; ++dz) {
            int cx = currentCenter.x + dx;
            int cz = currentCenter.z + dz;
            
            ChunkCoord coord = {cx, cz};
            if (m_Chunks.find(coord) == m_Chunks.end()) {
                LoadChunkAsync(cx, cz);
            }
        }
    }

    // Unload chunks outside unload radius
    std::vector<ChunkCoord> toUnload;
    for (const auto& [coord, chunk] : m_Chunks) {
        if (!ShouldBeLoaded(coord.x, coord.z, currentCenter)) {
            toUnload.push_back(coord);
        }
    }

    for (const auto& coord : toUnload) {
        UnloadChunk(coord.x, coord.z);
    }

    m_CenterChunk = currentCenter;
}

bool ChunkManager::ShouldBeLoaded(int chunkX, int chunkZ, const ChunkCoord& center) const {
    int dx = std::abs(chunkX - center.x);
    int dz = std::abs(chunkZ - center.z);
    return dx <= m_UnloadRadius && dz <= m_UnloadRadius;
}

void ChunkManager::LoadChunk(int chunkX, int chunkZ) {
    // Synchronous loading - for backwards compatibility or fallback
    auto chunk = std::make_unique<Chunk>();
    chunk->SetPosition(chunkX, chunkZ);
    chunk->SetState(ChunkState::Generating);
    
    // Generate terrain
    m_TerrainGenerator.Generate(*chunk);
    
    // Build heightmap for weather
    chunk->RebuildHeightmap();
    
    // Build mesh
    chunk->BuildMesh();
    chunk->SetState(ChunkState::Ready);
    
    ChunkCoord coord = {chunkX, chunkZ};
    m_Chunks[coord] = std::move(chunk);
    
    LOG_DEBUG("Sync loaded chunk (" + std::to_string(chunkX) + ", " + std::to_string(chunkZ) + 
              ") - Total: " + std::to_string(m_Chunks.size()));
}

void ChunkManager::LoadChunkAsync(int chunkX, int chunkZ) {
    // Create chunk immediately (for state tracking and map presence)
    auto chunk = std::make_unique<Chunk>();
    chunk->SetPosition(chunkX, chunkZ);
    chunk->SetState(ChunkState::Generating);
    
    Chunk* rawPtr = chunk.get();
    ChunkCoord coord = {chunkX, chunkZ};
    m_Chunks[coord] = std::move(chunk);
    
    // Submit task to thread pool (fire-and-forget style)
    m_ThreadPool.detach_task([this, rawPtr, chunkX, chunkZ]() {
        // Generate terrain (thread-safe read of config)
        m_TerrainGenerator.Generate(*rawPtr);
        
        // Build heightmap for weather
        rawPtr->RebuildHeightmap();
        
        rawPtr->SetState(ChunkState::Meshing);
        
        // Build mesh data without OpenGL calls
        ChunkMeshData meshData = rawPtr->GenerateMeshData();
        
        // Queue for main thread upload
        {
            std::lock_guard<std::mutex> lock(m_PendingMeshMutex);
            m_PendingMeshes.push(std::move(meshData));
        }
        
        rawPtr->SetState(ChunkState::MeshPending);
    });
}

void ChunkManager::ProcessPendingMeshes() {
    std::lock_guard<std::mutex> lock(m_PendingMeshMutex);
    
    int processed = 0;
    
    while (!m_PendingMeshes.empty() && processed < MAX_UPLOADS_PER_FRAME) {
        ChunkMeshData& data = m_PendingMeshes.front();
        
        Chunk* chunk = GetChunk(data.chunkX, data.chunkZ);
        if (chunk && chunk->GetState() == ChunkState::MeshPending) {
            chunk->UploadMeshFromData(data);
            chunk->SetState(ChunkState::Ready);
        }
        
        m_PendingMeshes.pop();
        processed++;
    }
}

void ChunkManager::UnloadChunk(int chunkX, int chunkZ) {
    ChunkCoord coord = {chunkX, chunkZ};
    auto it = m_Chunks.find(coord);
    if (it != m_Chunks.end()) {
        // Chunk destructor will cleanup mesh
        m_Chunks.erase(it);
        LOG_DEBUG("Unloaded chunk (" + std::to_string(chunkX) + ", " + std::to_string(chunkZ) + 
                  ") - Total: " + std::to_string(m_Chunks.size()));
    }
}

Chunk* ChunkManager::GetChunk(int chunkX, int chunkZ) {
    ChunkCoord coord = {chunkX, chunkZ};
    auto it = m_Chunks.find(coord);
    if (it != m_Chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

void ChunkManager::RenderAll(Core::Shader& shader, Core::Camera& camera, float aspectRatio) {
    shader.Bind();
    
    // Update frustum from current camera view
    camera.UpdateFrustum(aspectRatio);
    const Core::Frustum& frustum = camera.GetFrustum();
    
    // Pre-compute visible chunks with model matrices (reused by other passes)
    UpdateVisibleChunks(frustum);
    
    glm::mat4 viewProj = camera.GetViewProjectionMatrix(aspectRatio);
    
    for (const auto& visible : m_VisibleChunks) {
        glm::mat4 mvp = viewProj * visible.modelMatrix;
        
        shader.SetMat4("u_MVP", mvp);
        shader.SetMat4("u_Model", visible.modelMatrix);
        
        visible.chunk->Render();
    }
    
    shader.Unbind();
    
    // Debug logging (can be toggled off in production)
    static int frameCounter = 0;
    if (++frameCounter >= 60) {
        LOG_DEBUG("Frustum culling: rendered " + std::to_string(m_VisibleChunks.size()) + 
                  " chunks, culled " + std::to_string(m_Chunks.size() - m_VisibleChunks.size()));
        frameCounter = 0;
    }
}

void ChunkManager::RenderWater(Core::Shader& waterShader, Core::Camera& camera, float aspectRatio) {
    waterShader.Bind();
    
    // Reuse pre-computed visible chunks from RenderAll
    glm::mat4 viewProj = camera.GetViewProjectionMatrix(aspectRatio);
    
    for (const auto& visible : m_VisibleChunks) {
        if (!visible.hasWater) {
            continue;
        }
        
        glm::mat4 mvp = viewProj * visible.modelMatrix;
        
        waterShader.SetMat4("u_MVP", mvp);
        waterShader.SetMat4("u_Model", visible.modelMatrix);
        
        visible.chunk->RenderWater();
    }
    
    waterShader.Unbind();
}

void ChunkManager::RenderAllShadow(Core::Shader& shadowShader, const glm::mat4& lightSpaceMatrix) {
    shadowShader.Bind();
    shadowShader.SetMat4("u_LightSpaceMatrix", lightSpaceMatrix);
    
    // Reuse pre-computed visible chunks from RenderAll
    for (const auto& visible : m_VisibleChunks) {
        shadowShader.SetMat4("u_Model", visible.modelMatrix);
        visible.chunk->Render();
    }
    
    shadowShader.Unbind();
}

void ChunkManager::RenderAllDepth(Core::Shader& depthShader, const glm::mat4& view, const glm::mat4& projection) {
    depthShader.Bind();
    
    // Reuse pre-computed visible chunks from RenderAll
    for (const auto& visible : m_VisibleChunks) {
        glm::mat4 modelView = view * visible.modelMatrix;
        glm::mat4 mvp = projection * modelView;
        
        depthShader.SetMat4("u_MVP", mvp);
        depthShader.SetMat4("u_ModelView", modelView);
        
        visible.chunk->Render();
    }
    
    depthShader.Unbind();
}

const Chunk* ChunkManager::GetChunkConst(int chunkX, int chunkZ) const {
    ChunkCoord coord = {chunkX, chunkZ};
    auto it = m_Chunks.find(coord);
    if (it != m_Chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

void ChunkManager::UpdateVisibleChunks(const Core::Frustum& frustum) {
    m_VisibleChunks.clear();
    m_VisibleChunks.reserve(m_Chunks.size());
    
    for (const auto& [coord, chunk] : m_Chunks) {
        if (!chunk->HasMesh()) {
            continue;
        }
        
        // Calculate world bounds for this chunk
        float worldX = static_cast<float>(coord.x * CHUNK_WIDTH);
        float worldZ = static_cast<float>(coord.z * CHUNK_DEPTH);
        
        // Chunk AABB for frustum culling
        glm::vec3 minBounds(worldX, 0.0f, worldZ);
        glm::vec3 maxBounds(worldX + CHUNK_WIDTH, CHUNK_HEIGHT, worldZ + CHUNK_DEPTH);
        
        // Skip if chunk is outside frustum
        if (!frustum.IsAABBVisible(minBounds, maxBounds)) {
            continue;
        }
        
        // Pre-compute and cache model matrix
        VisibleChunk visible;
        visible.coord = coord;
        visible.chunk = chunk.get();
        visible.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(worldX, 0.0f, worldZ));
        visible.hasWater = chunk->HasWaterMesh();
        
        m_VisibleChunks.push_back(visible);
    }
}

BlockID ChunkManager::GetBlock(int worldX, int worldY, int worldZ) const {
    // Bounds check for Y (chunks extend from 0 to CHUNK_HEIGHT)
    if (worldY < 0 || worldY >= CHUNK_HEIGHT) {
        return BLOCK_AIR;
    }
    
    // Convert world coords to chunk coords using floor division
    int chunkX = (worldX >= 0) ? (worldX / CHUNK_WIDTH) : ((worldX + 1) / CHUNK_WIDTH - 1);
    int chunkZ = (worldZ >= 0) ? (worldZ / CHUNK_DEPTH) : ((worldZ + 1) / CHUNK_DEPTH - 1);
    
    // Local coords within chunk
    int localX = worldX - chunkX * CHUNK_WIDTH;
    int localZ = worldZ - chunkZ * CHUNK_DEPTH;
    
    // Find chunk (const access)
    const Chunk* chunk = GetChunkConst(chunkX, chunkZ);
    if (!chunk) {
        return BLOCK_AIR;
    }
    
    // Get block from chunk (BlockType is compatible with BlockID via uint16_t)
    return static_cast<BlockID>(chunk->GetBlock(localX, worldY, localZ));
}

void ChunkManager::SetBlock(int worldX, int worldY, int worldZ, BlockID block) {
    // Bounds check for Y
    if (worldY < 0 || worldY >= CHUNK_HEIGHT) {
        return;
    }
    
    // Convert world coords to chunk coords using floor division
    int chunkX = (worldX >= 0) ? (worldX / CHUNK_WIDTH) : ((worldX + 1) / CHUNK_WIDTH - 1);
    int chunkZ = (worldZ >= 0) ? (worldZ / CHUNK_DEPTH) : ((worldZ + 1) / CHUNK_DEPTH - 1);
    
    // Local coords within chunk
    int localX = worldX - chunkX * CHUNK_WIDTH;
    int localZ = worldZ - chunkZ * CHUNK_DEPTH;
    
    // Find chunk
    Chunk* chunk = GetChunk(chunkX, chunkZ);
    if (!chunk) {
        return;
    }
    
    // Set block in chunk (convert BlockID to BlockType)
    chunk->SetBlock(localX, worldY, localZ, static_cast<BlockType>(block));
    
    // Rebuild heightmap for this column
    chunk->RebuildHeightmap();
    
    // Rebuild this chunk's mesh
    RebuildChunkMesh(chunkX, chunkZ);
    
    // Check if we need to rebuild neighbor chunks (block on boundary)
    if (localX == 0) {
        RebuildChunkMesh(chunkX - 1, chunkZ);
    } else if (localX == CHUNK_WIDTH - 1) {
        RebuildChunkMesh(chunkX + 1, chunkZ);
    }
    
    if (localZ == 0) {
        RebuildChunkMesh(chunkX, chunkZ - 1);
    } else if (localZ == CHUNK_DEPTH - 1) {
        RebuildChunkMesh(chunkX, chunkZ + 1);
    }
}

void ChunkManager::RebuildChunkMesh(int chunkX, int chunkZ) {
    Chunk* chunk = GetChunk(chunkX, chunkZ);
    if (!chunk || chunk->GetState() != ChunkState::Ready) {
        return;
    }
    
    // Mark chunk as needing rebuild
    chunk->SetState(ChunkState::Meshing);
    
    // Submit async mesh rebuild task
    Chunk* rawPtr = chunk;
    m_ThreadPool.detach_task([this, rawPtr, chunkX, chunkZ]() {
        // Build mesh data without OpenGL calls (thread-safe)
        ChunkMeshData meshData = rawPtr->GenerateMeshData();
        
        // Queue for main thread upload
        {
            std::lock_guard<std::mutex> lock(m_PendingMeshMutex);
            m_PendingMeshes.push(std::move(meshData));
        }
        
        rawPtr->SetState(ChunkState::MeshPending);
    });
}

int ChunkManager::GetHeightAt(int worldX, int worldZ) const {
    // Convert world coords to chunk coords using floor division
    int chunkX = (worldX >= 0) ? (worldX / CHUNK_WIDTH) : ((worldX + 1) / CHUNK_WIDTH - 1);
    int chunkZ = (worldZ >= 0) ? (worldZ / CHUNK_DEPTH) : ((worldZ + 1) / CHUNK_DEPTH - 1);
    
    // Local coords within chunk
    int localX = worldX - chunkX * CHUNK_WIDTH;
    int localZ = worldZ - chunkZ * CHUNK_DEPTH;
    
    // Find chunk
    const Chunk* chunk = GetChunkConst(chunkX, chunkZ);
    if (!chunk) {
        return -1;
    }
    
    return chunk->GetHeightAt(localX, localZ);
}

} // namespace Voxel
