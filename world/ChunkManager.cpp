#include "ChunkManager.h"
#include "core/Shader.h"
#include "core/Camera.h"
#include "core/Frustum.h"
#include "core/Logger.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace Voxel {

ChunkManager::ChunkManager() {
    // Configure terrain generator with default settings
    TerrainConfig config;
    config.seed = 12345;
    config.frequency = 0.02f;
    config.baseHeight = 64;
    config.amplitude = 20;
    m_TerrainGenerator.SetConfig(config);
    
    LOG_INFO("ChunkManager initialized with load radius " + std::to_string(m_LoadRadius));
}

ChunkCoord ChunkManager::WorldToChunkCoord(const glm::vec3& worldPos) const {
    // Floor division to get chunk coordinates
    int chunkX = static_cast<int>(std::floor(worldPos.x / CHUNK_WIDTH));
    int chunkZ = static_cast<int>(std::floor(worldPos.z / CHUNK_DEPTH));
    return {chunkX, chunkZ};
}

void ChunkManager::Update(const glm::vec3& playerPos) {
    ChunkCoord currentCenter = WorldToChunkCoord(playerPos);

    // Load chunks within load radius
    for (int dx = -m_LoadRadius; dx <= m_LoadRadius; ++dx) {
        for (int dz = -m_LoadRadius; dz <= m_LoadRadius; ++dz) {
            int cx = currentCenter.x + dx;
            int cz = currentCenter.z + dz;
            
            ChunkCoord coord = {cx, cz};
            if (m_Chunks.find(coord) == m_Chunks.end()) {
                LoadChunk(cx, cz);
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
    auto chunk = std::make_unique<Chunk>();
    chunk->SetPosition(chunkX, chunkZ);
    
    // Generate terrain
    m_TerrainGenerator.Generate(*chunk);
    
    // Build mesh
    chunk->BuildMesh();
    
    ChunkCoord coord = {chunkX, chunkZ};
    m_Chunks[coord] = std::move(chunk);
    
    LOG_DEBUG("Loaded chunk (" + std::to_string(chunkX) + ", " + std::to_string(chunkZ) + 
              ") - Total: " + std::to_string(m_Chunks.size()));
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

} // namespace Voxel
