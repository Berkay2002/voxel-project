#include <gtest/gtest.h>
#include "world/VoxelRaycast.h"
#include "world/ChunkManager.h"
#include "world/BlockRegistry.h"
#include "core/scene/Ray.h"
#include <glm/glm.hpp>

using namespace Voxel;
using namespace Core;

/**
 * Test fixture for VoxelRaycast tests
 * Note: These tests use ChunkManager which includes world generation.
 * We test the DDA algorithm logic without relying on specific terrain.
 */
class VoxelRaycastTest : public ::testing::Test {
protected:
    ChunkManager* chunkManager;
    
    void SetUp() override {
        // Create chunk manager
        chunkManager = new ChunkManager();
        
        // Initialize the BlockRegistry with a minimal configuration
        // This ensures BlockRegistry is ready for IsSolid() checks
        BlockRegistry::Instance().Clear();
    }
    
    void TearDown() override {
        delete chunkManager;
        BlockRegistry::Instance().Clear();
    }
    
    // Helper to place a test block at world coordinates
    void PlaceBlock(int x, int y, int z, BlockType block) {
        chunkManager->SetBlock(x, y, z, ToBlockID(block));
    }
    
    // Helper to verify basic raycast properties
    bool IsRaycastValid(const RaycastResult& result) {
        if (!result.hit) return true;
        return result.distance >= 0.0f;
    }
};

/**
 * Test RaycastResult default state
 */
TEST_F(VoxelRaycastTest, RaycastResultDefault) {
    RaycastResult result;
    
    EXPECT_FALSE(result.hit);
    EXPECT_EQ(result.blockPos, glm::ivec3(0));
    EXPECT_EQ(result.previousPos, glm::ivec3(0));
    EXPECT_EQ(result.blockType, BLOCK_AIR);
    EXPECT_FLOAT_EQ(result.distance, 0.0f);
}

/**
 * Test raycast with no blocks (should miss)
 */
TEST_F(VoxelRaycastTest, RaycastNoBlocks) {
    // Cast ray through empty space
    Ray ray(glm::vec3(0.0f, 100.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    RaycastResult result = Raycast(ray, *chunkManager, 10.0f);
    
    // Should not hit anything in empty space
    EXPECT_FALSE(result.hit);
    EXPECT_TRUE(IsRaycastValid(result));
}

/**
 * Test raycast hitting a single block
 */
TEST_F(VoxelRaycastTest, RaycastSingleBlock) {
    // Place a block at (5, 64, 5)
    PlaceBlock(5, 64, 5, BlockType::Stone);
    
    // Cast ray from (0, 64, 5) towards the block
    Ray ray(glm::vec3(0.0f, 64.5f, 5.5f), glm::vec3(1.0f, 0.0f, 0.0f));
    RaycastResult result = Raycast(ray, *chunkManager, 10.0f);
    
    // Should hit the block
    EXPECT_TRUE(result.hit);
    EXPECT_EQ(result.blockPos, glm::ivec3(5, 64, 5));
    EXPECT_TRUE(IsRaycastValid(result));
}

/**
 * Test raycast maximum distance
 */
TEST_F(VoxelRaycastTest, RaycastMaxDistance) {
    // Place a block far away
    PlaceBlock(20, 64, 5, BlockType::Stone);
    
    // Cast ray with short max distance
    Ray ray(glm::vec3(0.0f, 64.5f, 5.5f), glm::vec3(1.0f, 0.0f, 0.0f));
    RaycastResult result = Raycast(ray, *chunkManager, 5.0f);
    
    // Should not reach the block
    EXPECT_FALSE(result.hit);
}

/**
 * Test raycast direction normalization
 */
TEST_F(VoxelRaycastTest, RaycastDirectionNormalization) {
    PlaceBlock(3, 64, 5, BlockType::Stone);
    
    // Create ray with unnormalized direction (Ray constructor normalizes it)
    Ray ray(glm::vec3(0.0f, 64.5f, 5.5f), glm::vec3(10.0f, 0.0f, 0.0f));
    RaycastResult result = Raycast(ray, *chunkManager, 10.0f);
    
    EXPECT_TRUE(result.hit);
    EXPECT_EQ(result.blockPos, glm::ivec3(3, 64, 5));
}

/**
 * Test raycast along Y axis (vertical)
 */
TEST_F(VoxelRaycastTest, RaycastVertical) {
    // Place a block above
    PlaceBlock(5, 100, 5, BlockType::Stone);
    
    // Cast ray upward from below
    Ray ray(glm::vec3(5.5f, 50.0f, 5.5f), glm::vec3(0.0f, 1.0f, 0.0f));
    RaycastResult result = Raycast(ray, *chunkManager, 60.0f);
    
    EXPECT_TRUE(result.hit);
    EXPECT_EQ(result.blockPos, glm::ivec3(5, 100, 5));
    EXPECT_EQ(result.hitFace, Face::Bottom); // Entering from bottom
}

/**
 * Test raycast along Z axis
 */
TEST_F(VoxelRaycastTest, RaycastZAxis) {
    // Place a block ahead
    PlaceBlock(5, 64, 10, BlockType::Stone);
    
    // Cast ray forward along Z
    Ray ray(glm::vec3(5.5f, 64.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    RaycastResult result = Raycast(ray, *chunkManager, 15.0f);
    
    EXPECT_TRUE(result.hit);
    EXPECT_EQ(result.blockPos, glm::ivec3(5, 64, 10));
    EXPECT_EQ(result.hitFace, Face::South); // Entering from south (-Z)
}

/**
 * Test raycast diagonal
 */
TEST_F(VoxelRaycastTest, RaycastDiagonal) {
    // Place a block diagonally
    PlaceBlock(5, 65, 5, BlockType::Stone);
    
    // Cast ray diagonally
    glm::vec3 direction = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
    Ray ray(glm::vec3(0.0f, 60.0f, 0.0f), direction);
    RaycastResult result = Raycast(ray, *chunkManager, 10.0f);
    
    // Should hit the block
    EXPECT_TRUE(result.hit);
    EXPECT_TRUE(IsRaycastValid(result));
}

/**
 * Test raycast with negative direction
 */
TEST_F(VoxelRaycastTest, RaycastNegativeDirection) {
    // Place a block behind the ray origin
    PlaceBlock(-5, 64, 5, BlockType::Stone);
    
    // Cast ray backward (negative X)
    Ray ray(glm::vec3(0.0f, 64.5f, 5.5f), glm::vec3(-1.0f, 0.0f, 0.0f));
    RaycastResult result = Raycast(ray, *chunkManager, 10.0f);
    
    EXPECT_TRUE(result.hit);
    EXPECT_EQ(result.blockPos, glm::ivec3(-5, 64, 5));
    EXPECT_EQ(result.hitFace, Face::East); // Entering from east (+X side)
}

/**
 * Test raycast result contains correct face information
 */
TEST_F(VoxelRaycastTest, RaycastHitFace) {
    PlaceBlock(0, 64, 0, BlockType::Stone);
    
    // Hit from west side (-X)
    Ray rayWest(glm::vec3(-5.0f, 64.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f));
    RaycastResult resultWest = Raycast(rayWest, *chunkManager, 10.0f);
    EXPECT_TRUE(resultWest.hit);
    EXPECT_EQ(resultWest.hitFace, Face::West);
    
    // Hit from top side (+Y)
    Ray rayTop(glm::vec3(0.5f, 70.0f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f));
    RaycastResult resultTop = Raycast(rayTop, *chunkManager, 10.0f);
    EXPECT_TRUE(resultTop.hit);
    EXPECT_EQ(resultTop.hitFace, Face::Top);
}

/**
 * Test raycast previousPos for block placement
 */
TEST_F(VoxelRaycastTest, RaycastPreviousPosition) {
    PlaceBlock(5, 64, 5, BlockType::Stone);
    
    // Cast ray from west
    Ray ray(glm::vec3(0.0f, 64.5f, 5.5f), glm::vec3(1.0f, 0.0f, 0.0f));
    RaycastResult result = Raycast(ray, *chunkManager, 10.0f);
    
    EXPECT_TRUE(result.hit);
    EXPECT_EQ(result.blockPos, glm::ivec3(5, 64, 5));
    
    // Previous position should be one block before in the ray direction
    EXPECT_EQ(result.previousPos, glm::ivec3(4, 64, 5));
}

/**
 * Test raycast through multiple blocks (should hit first)
 */
TEST_F(VoxelRaycastTest, RaycastMultipleBlocks) {
    // Place blocks in a line
    PlaceBlock(3, 64, 5, BlockType::Stone);
    PlaceBlock(5, 64, 5, BlockType::Dirt);
    PlaceBlock(7, 64, 5, BlockType::Grass);
    
    // Cast ray through all of them
    Ray ray(glm::vec3(0.0f, 64.5f, 5.5f), glm::vec3(1.0f, 0.0f, 0.0f));
    RaycastResult result = Raycast(ray, *chunkManager, 20.0f);
    
    // Should hit the first block
    EXPECT_TRUE(result.hit);
    EXPECT_EQ(result.blockPos, glm::ivec3(3, 64, 5));
    EXPECT_EQ(result.blockType, ToBlockID(BlockType::Stone));
}
