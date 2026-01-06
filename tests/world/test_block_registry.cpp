#include <gtest/gtest.h>
#include "world/BlockRegistry.h"
#include "core/rendering/TextureManager.h"
#include <fstream>
#include <filesystem>

using namespace Voxel;

/**
 * Test fixture for BlockRegistry tests
 */
class BlockRegistryTest : public ::testing::Test {
protected:
    std::string testConfigPath;
    
    void SetUp() override {
        // Create a temporary test JSON config
        testConfigPath = "/tmp/test_blocks.json";
        CreateTestBlockConfig();
    }
    
    void TearDown() override {
        // Clean up
        BlockRegistry::Instance().Clear();
        Core::TextureManager::Instance().Clear();
        if (std::filesystem::exists(testConfigPath)) {
            std::filesystem::remove(testConfigPath);
        }
    }
    
    void CreateTestBlockConfig() {
        // Create a minimal blocks.json for testing
        std::ofstream file(testConfigPath);
        file << R"({
  "blocks": [
    {
      "id": 0,
      "stringId": "air",
      "solid": false,
      "opaque": false,
      "transparent": false
    },
    {
      "id": 1,
      "stringId": "stone",
      "solid": true,
      "opaque": true,
      "transparent": false,
      "tintColor": [0.5, 0.5, 0.5],
      "textures": {
        "all": "stone"
      }
    },
    {
      "id": 2,
      "stringId": "grass_block",
      "solid": true,
      "opaque": true,
      "transparent": false,
      "tinted": true,
      "tintTop": true,
      "tintSides": true,
      "tintColor": [0.4, 0.8, 0.3],
      "textures": {
        "top": "grass_top",
        "bottom": "dirt",
        "sides": "grass_side"
      }
    },
    {
      "id": 3,
      "stringId": "water",
      "solid": true,
      "opaque": false,
      "transparent": true,
      "tintColor": [0.2, 0.4, 1.0],
      "textures": {
        "all": "water"
      }
    }
  ]
})";
        file.close();
    }
};

/**
 * Test BlockRegistry singleton
 */
TEST_F(BlockRegistryTest, Singleton) {
    BlockRegistry& registry1 = BlockRegistry::Instance();
    BlockRegistry& registry2 = BlockRegistry::Instance();
    
    // Both references should point to the same instance
    EXPECT_EQ(&registry1, &registry2);
}

/**
 * Test loading blocks from JSON
 */
TEST_F(BlockRegistryTest, LoadFromFile) {
    BlockRegistry& registry = BlockRegistry::Instance();
    Core::TextureManager& textureManager = Core::TextureManager::Instance();
    
    bool loaded = registry.LoadFromFile(testConfigPath, textureManager);
    EXPECT_TRUE(loaded);
    EXPECT_TRUE(registry.IsLoaded());
    
    // Should have 4 blocks (air + 3 test blocks)
    EXPECT_EQ(registry.GetBlockCount(), 4);
}

/**
 * Test getting block ID by string
 */
TEST_F(BlockRegistryTest, GetBlockIDByString) {
    BlockRegistry& registry = BlockRegistry::Instance();
    Core::TextureManager& textureManager = Core::TextureManager::Instance();
    registry.LoadFromFile(testConfigPath, textureManager);
    
    EXPECT_EQ(registry.GetBlockID("air"), 0);
    EXPECT_EQ(registry.GetBlockID("stone"), 1);
    EXPECT_EQ(registry.GetBlockID("grass_block"), 2);
    EXPECT_EQ(registry.GetBlockID("water"), 3);
    
    // Non-existent block should return BLOCK_INVALID
    EXPECT_EQ(registry.GetBlockID("nonexistent"), BLOCK_INVALID);
}

/**
 * Test getting block definition by ID
 */
TEST_F(BlockRegistryTest, GetBlockDefByID) {
    BlockRegistry& registry = BlockRegistry::Instance();
    Core::TextureManager& textureManager = Core::TextureManager::Instance();
    registry.LoadFromFile(testConfigPath, textureManager);
    
    const BlockDef& air = registry.GetBlockDef(0);
    EXPECT_EQ(air.stringId, "air");
    EXPECT_FALSE(air.solid);
    EXPECT_FALSE(air.opaque);
    
    const BlockDef& stone = registry.GetBlockDef(1);
    EXPECT_EQ(stone.stringId, "stone");
    EXPECT_TRUE(stone.solid);
    EXPECT_TRUE(stone.opaque);
    
    const BlockDef& grass = registry.GetBlockDef(2);
    EXPECT_EQ(grass.stringId, "grass_block");
    EXPECT_TRUE(grass.tinted);
    EXPECT_TRUE(grass.tintTop);
    EXPECT_TRUE(grass.tintSides);
}

/**
 * Test block property queries
 */
TEST_F(BlockRegistryTest, BlockProperties) {
    BlockRegistry& registry = BlockRegistry::Instance();
    Core::TextureManager& textureManager = Core::TextureManager::Instance();
    registry.LoadFromFile(testConfigPath, textureManager);
    
    // Air
    EXPECT_TRUE(registry.IsAir(0));
    EXPECT_FALSE(registry.IsSolid(0));
    EXPECT_FALSE(registry.IsOpaque(0));
    EXPECT_FALSE(registry.IsTransparent(0));
    
    // Stone
    EXPECT_FALSE(registry.IsAir(1));
    EXPECT_TRUE(registry.IsSolid(1));
    EXPECT_TRUE(registry.IsOpaque(1));
    EXPECT_FALSE(registry.IsTransparent(1));
    
    // Water
    EXPECT_FALSE(registry.IsAir(3));
    EXPECT_TRUE(registry.IsSolid(3));
    EXPECT_FALSE(registry.IsOpaque(3));
    EXPECT_TRUE(registry.IsTransparent(3));
}

/**
 * Test tint color retrieval
 */
TEST_F(BlockRegistryTest, TintColors) {
    BlockRegistry& registry = BlockRegistry::Instance();
    Core::TextureManager& textureManager = Core::TextureManager::Instance();
    registry.LoadFromFile(testConfigPath, textureManager);
    
    // Stone should have a gray tint
    glm::vec3 stoneTint = registry.GetTintColor(1, Face::Top);
    EXPECT_FLOAT_EQ(stoneTint.r, 0.5f);
    EXPECT_FLOAT_EQ(stoneTint.g, 0.5f);
    EXPECT_FLOAT_EQ(stoneTint.b, 0.5f);
    
    // Grass top should be tinted green
    glm::vec3 grassTopTint = registry.GetTintColor(2, Face::Top);
    EXPECT_FLOAT_EQ(grassTopTint.r, 0.4f);
    EXPECT_FLOAT_EQ(grassTopTint.g, 0.8f);
    EXPECT_FLOAT_EQ(grassTopTint.b, 0.3f);
    
    // Grass sides should also be tinted
    glm::vec3 grassSideTint = registry.GetTintColor(2, Face::North);
    EXPECT_FLOAT_EQ(grassSideTint.r, 0.4f);
    EXPECT_FLOAT_EQ(grassSideTint.g, 0.8f);
    EXPECT_FLOAT_EQ(grassSideTint.b, 0.3f);
}

/**
 * Test face direction helper
 */
TEST_F(BlockRegistryTest, FaceDirection) {
    glm::ivec3 topDir = GetFaceDirection(Face::Top);
    EXPECT_EQ(topDir, glm::ivec3(0, 1, 0));
    
    glm::ivec3 bottomDir = GetFaceDirection(Face::Bottom);
    EXPECT_EQ(bottomDir, glm::ivec3(0, -1, 0));
    
    glm::ivec3 northDir = GetFaceDirection(Face::North);
    EXPECT_EQ(northDir, glm::ivec3(0, 0, 1));
    
    glm::ivec3 southDir = GetFaceDirection(Face::South);
    EXPECT_EQ(southDir, glm::ivec3(0, 0, -1));
    
    glm::ivec3 eastDir = GetFaceDirection(Face::East);
    EXPECT_EQ(eastDir, glm::ivec3(1, 0, 0));
    
    glm::ivec3 westDir = GetFaceDirection(Face::West);
    EXPECT_EQ(westDir, glm::ivec3(-1, 0, 0));
}

/**
 * Test clearing the registry
 */
TEST_F(BlockRegistryTest, Clear) {
    BlockRegistry& registry = BlockRegistry::Instance();
    Core::TextureManager& textureManager = Core::TextureManager::Instance();
    registry.LoadFromFile(testConfigPath, textureManager);
    
    EXPECT_TRUE(registry.IsLoaded());
    EXPECT_GT(registry.GetBlockCount(), 0);
    
    registry.Clear();
    
    EXPECT_FALSE(registry.IsLoaded());
    EXPECT_EQ(registry.GetBlockCount(), 0);
}

/**
 * Test loading invalid file
 */
TEST_F(BlockRegistryTest, LoadInvalidFile) {
    BlockRegistry& registry = BlockRegistry::Instance();
    Core::TextureManager& textureManager = Core::TextureManager::Instance();
    
    bool loaded = registry.LoadFromFile("/nonexistent/path/blocks.json", textureManager);
    EXPECT_FALSE(loaded);
    EXPECT_FALSE(registry.IsLoaded());
}

/**
 * Test helper functions (non-member)
 */
TEST_F(BlockRegistryTest, HelperFunctions) {
    BlockRegistry& registry = BlockRegistry::Instance();
    Core::TextureManager& textureManager = Core::TextureManager::Instance();
    registry.LoadFromFile(testConfigPath, textureManager);
    
    // Test global helper functions
    EXPECT_TRUE(IsOpaque(1));  // Stone
    EXPECT_FALSE(IsOpaque(3)); // Water
    
    EXPECT_TRUE(IsSolid(1));   // Stone
    EXPECT_FALSE(IsSolid(0));  // Air
    
    EXPECT_TRUE(IsTransparent(3));  // Water
    EXPECT_FALSE(IsTransparent(1)); // Stone
}

/**
 * Test special block ID constants
 */
TEST_F(BlockRegistryTest, SpecialBlockIDs) {
    EXPECT_EQ(BLOCK_AIR, 0);
    EXPECT_EQ(BLOCK_INVALID, 65535);
}
