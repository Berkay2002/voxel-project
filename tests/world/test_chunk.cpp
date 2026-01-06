#include <gtest/gtest.h>
#include "world/Chunk.h"

using namespace Voxel;

/**
 * Test fixture for Chunk tests
 */
class ChunkTest : public ::testing::Test {
protected:
    Chunk chunk;
    
    void SetUp() override {
        chunk.SetPosition(0, 0);
    }
};

/**
 * Test chunk dimensions constants
 */
TEST_F(ChunkTest, ChunkDimensions) {
    EXPECT_EQ(CHUNK_WIDTH, 16);
    EXPECT_EQ(CHUNK_HEIGHT, 256);
    EXPECT_EQ(CHUNK_DEPTH, 16);
    EXPECT_EQ(CHUNK_VOLUME, 16 * 256 * 16);
}

/**
 * Test chunk position get/set
 */
TEST_F(ChunkTest, ChunkPosition) {
    chunk.SetPosition(5, 7);
    EXPECT_EQ(chunk.GetChunkX(), 5);
    EXPECT_EQ(chunk.GetChunkZ(), 7);
    
    chunk.SetPosition(-3, -10);
    EXPECT_EQ(chunk.GetChunkX(), -3);
    EXPECT_EQ(chunk.GetChunkZ(), -10);
}

/**
 * Test bounds checking - valid coordinates
 */
TEST_F(ChunkTest, IsInBoundsValid) {
    // Corner cases
    EXPECT_TRUE(chunk.IsInBounds(0, 0, 0));
    EXPECT_TRUE(chunk.IsInBounds(15, 255, 15));
    
    // Middle of chunk
    EXPECT_TRUE(chunk.IsInBounds(8, 128, 8));
}

/**
 * Test bounds checking - invalid coordinates
 */
TEST_F(ChunkTest, IsInBoundsInvalid) {
    // Out of bounds on X
    EXPECT_FALSE(chunk.IsInBounds(-1, 0, 0));
    EXPECT_FALSE(chunk.IsInBounds(16, 0, 0));
    
    // Out of bounds on Y
    EXPECT_FALSE(chunk.IsInBounds(0, -1, 0));
    EXPECT_FALSE(chunk.IsInBounds(0, 256, 0));
    
    // Out of bounds on Z
    EXPECT_FALSE(chunk.IsInBounds(0, 0, -1));
    EXPECT_FALSE(chunk.IsInBounds(0, 0, 16));
}

/**
 * Test setting and getting blocks
 */
TEST_F(ChunkTest, SetAndGetBlock) {
    // Initially should be air (0)
    EXPECT_EQ(chunk.GetBlock(5, 64, 7), BlockType::Air);
    
    // Set a block
    chunk.SetBlock(5, 64, 7, BlockType::Stone);
    EXPECT_EQ(chunk.GetBlock(5, 64, 7), BlockType::Stone);
    
    // Set another block
    chunk.SetBlock(0, 0, 0, BlockType::Dirt);
    EXPECT_EQ(chunk.GetBlock(0, 0, 0), BlockType::Dirt);
}

/**
 * Test that blocks don't interfere with each other
 */
TEST_F(ChunkTest, BlockIsolation) {
    chunk.SetBlock(0, 0, 0, BlockType::Grass);
    chunk.SetBlock(1, 0, 0, BlockType::Dirt);
    chunk.SetBlock(0, 1, 0, BlockType::Stone);
    chunk.SetBlock(0, 0, 1, BlockType::Water);
    
    EXPECT_EQ(chunk.GetBlock(0, 0, 0), BlockType::Grass);
    EXPECT_EQ(chunk.GetBlock(1, 0, 0), BlockType::Dirt);
    EXPECT_EQ(chunk.GetBlock(0, 1, 0), BlockType::Stone);
    EXPECT_EQ(chunk.GetBlock(0, 0, 1), BlockType::Water);
    
    // All other blocks should still be air
    EXPECT_EQ(chunk.GetBlock(2, 0, 0), BlockType::Air);
    EXPECT_EQ(chunk.GetBlock(0, 2, 0), BlockType::Air);
}

/**
 * Test dirty flag
 */
TEST_F(ChunkTest, DirtyFlag) {
    // Chunks start dirty
    EXPECT_TRUE(chunk.IsDirty());
    
    chunk.SetDirty(false);
    EXPECT_FALSE(chunk.IsDirty());
    
    chunk.SetDirty(true);
    EXPECT_TRUE(chunk.IsDirty());
}

/**
 * Test chunk state management
 */
TEST_F(ChunkTest, ChunkState) {
    // Default state should be Unloaded
    EXPECT_EQ(chunk.GetState(), ChunkState::Unloaded);
    
    chunk.SetState(ChunkState::Generating);
    EXPECT_EQ(chunk.GetState(), ChunkState::Generating);
    
    chunk.SetState(ChunkState::MeshPending);
    EXPECT_EQ(chunk.GetState(), ChunkState::MeshPending);
    
    chunk.SetState(ChunkState::Ready);
    EXPECT_EQ(chunk.GetState(), ChunkState::Ready);
}

/**
 * Test heightmap functionality
 */
TEST_F(ChunkTest, Heightmap) {
    // Fill a column with blocks
    for (int y = 0; y < 64; y++) {
        chunk.SetBlock(8, y, 8, BlockType::Stone);
    }
    
    // Rebuild heightmap
    chunk.RebuildHeightmap();
    
    // The highest solid block should be at Y=63
    EXPECT_EQ(chunk.GetHeightAt(8, 8), 63);
    
    // Adjacent column should be empty
    EXPECT_EQ(chunk.GetHeightAt(9, 8), -1);
}

/**
 * Test heightmap with mixed solid/air blocks
 */
TEST_F(ChunkTest, HeightmapWithGaps) {
    // Create a column with gaps
    chunk.SetBlock(5, 10, 5, BlockType::Stone);
    chunk.SetBlock(5, 20, 5, BlockType::Stone);
    chunk.SetBlock(5, 30, 5, BlockType::Stone);
    
    chunk.RebuildHeightmap();
    
    // Should return the highest solid block
    EXPECT_EQ(chunk.GetHeightAt(5, 5), 30);
}

/**
 * Test heightmap for empty column
 */
TEST_F(ChunkTest, HeightmapEmptyColumn) {
    // Don't place any blocks
    chunk.RebuildHeightmap();
    
    // All columns should return -1 (no solid blocks)
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int z = 0; z < CHUNK_DEPTH; z++) {
            EXPECT_EQ(chunk.GetHeightAt(x, z), -1);
        }
    }
}

/**
 * Test heightmap at chunk boundaries
 */
TEST_F(ChunkTest, HeightmapBoundaries) {
    // Test corner blocks
    chunk.SetBlock(0, 50, 0, BlockType::Stone);
    chunk.SetBlock(15, 60, 0, BlockType::Stone);
    chunk.SetBlock(0, 70, 15, BlockType::Stone);
    chunk.SetBlock(15, 80, 15, BlockType::Stone);
    
    chunk.RebuildHeightmap();
    
    EXPECT_EQ(chunk.GetHeightAt(0, 0), 50);
    EXPECT_EQ(chunk.GetHeightAt(15, 0), 60);
    EXPECT_EQ(chunk.GetHeightAt(0, 15), 70);
    EXPECT_EQ(chunk.GetHeightAt(15, 15), 80);
}

/**
 * Test setting blocks at maximum height
 */
TEST_F(ChunkTest, MaxHeightBlocks) {
    chunk.SetBlock(7, 255, 7, BlockType::Stone);
    EXPECT_EQ(chunk.GetBlock(7, 255, 7), BlockType::Stone);
    
    chunk.RebuildHeightmap();
    EXPECT_EQ(chunk.GetHeightAt(7, 7), 255);
}
