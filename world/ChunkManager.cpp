#include "ChunkManager.h"
#include "SpaghettiCaveCarver.h"
#include "core/Shader.h"
#include "core/Camera.h"
#include "core/Frustum.h"
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
    
    glm::mat4 viewProj = camera.GetViewProjectionMatrix(aspectRatio);
    
    int renderedCount = 0;
    int culledCount = 0;
    
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
        
        // Skip rendering if chunk is outside frustum
        if (!frustum.IsAABBVisible(minBounds, maxBounds)) {
            culledCount++;
            continue;
        }
        
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(worldX, 0.0f, worldZ));
        glm::mat4 mvp = viewProj * model;
        
        shader.SetMat4("u_MVP", mvp);
        shader.SetMat4("u_Model", model);  // For normal transformation
        
        chunk->Render();
        renderedCount++;
    }
    
    shader.Unbind();
    
    // Debug logging (can be toggled off in production)
    static int frameCounter = 0;
    if (++frameCounter >= 60) {  // Log every 60 frames
        LOG_DEBUG("Frustum culling: rendered " + std::to_string(renderedCount) + 
                  ", culled " + std::to_string(culledCount) + " chunks");
        frameCounter = 0;
    }
}

void ChunkManager::RenderWater(Core::Shader& waterShader, Core::Camera& camera, float aspectRatio) {
    waterShader.Bind();
    
    // Frustum should already be updated from RenderAll, but update just in case
    const Core::Frustum& frustum = camera.GetFrustum();
    
    glm::mat4 viewProj = camera.GetViewProjectionMatrix(aspectRatio);
    
    for (const auto& [coord, chunk] : m_Chunks) {
        if (!chunk->HasWaterMesh()) {
            continue;
        }

        // Calculate world bounds for this chunk
        float worldX = static_cast<float>(coord.x * CHUNK_WIDTH);
        float worldZ = static_cast<float>(coord.z * CHUNK_DEPTH);
        
        // Chunk AABB for frustum culling
        glm::vec3 minBounds(worldX, 0.0f, worldZ);
        glm::vec3 maxBounds(worldX + CHUNK_WIDTH, CHUNK_HEIGHT, worldZ + CHUNK_DEPTH);
        
        // Skip rendering if chunk is outside frustum
        if (!frustum.IsAABBVisible(minBounds, maxBounds)) {
            continue;
        }
        
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(worldX, 0.0f, worldZ));
        glm::mat4 mvp = viewProj * model;
        
        waterShader.SetMat4("u_MVP", mvp);
        waterShader.SetMat4("u_Model", model);
        
        chunk->RenderWater();
    }
    
    waterShader.Unbind();
}

void ChunkManager::RenderAllShadow(Core::Shader& shadowShader, const glm::mat4& lightSpaceMatrix) {
    shadowShader.Bind();
    shadowShader.SetMat4("u_LightSpaceMatrix", lightSpaceMatrix);
    
    for (const auto& [coord, chunk] : m_Chunks) {
        if (!chunk->HasMesh()) {
            continue;
        }
        
        // Calculate world position for this chunk
        float worldX = static_cast<float>(coord.x * CHUNK_WIDTH);
        float worldZ = static_cast<float>(coord.z * CHUNK_DEPTH);
        
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(worldX, 0.0f, worldZ));
        
        shadowShader.SetMat4("u_Model", model);
        
        // Render only opaque geometry for shadows (water doesn't cast shadows)
        chunk->Render();
    }
    
    shadowShader.Unbind();
}

void ChunkManager::RenderAllDepth(Core::Shader& depthShader, const glm::mat4& view, const glm::mat4& projection) {
    depthShader.Bind();
    
    for (const auto& [coord, chunk] : m_Chunks) {
        if (!chunk->HasMesh()) {
            continue;
        }
        
        // Calculate world position for this chunk
        float worldX = static_cast<float>(coord.x * CHUNK_WIDTH);
        float worldZ = static_cast<float>(coord.z * CHUNK_DEPTH);
        
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(worldX, 0.0f, worldZ));
        glm::mat4 modelView = view * model;
        glm::mat4 mvp = projection * modelView;
        
        depthShader.SetMat4("u_MVP", mvp);
        depthShader.SetMat4("u_ModelView", modelView);
        
        // Render only opaque geometry (no water for SSAO)
        chunk->Render();
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

} // namespace Voxel
