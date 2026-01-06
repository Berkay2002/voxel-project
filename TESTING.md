# Adding New Tests

This guide explains how to add new unit tests to the Voxel Engine project.

## Test Structure

Tests are organized in the `tests/` directory, mirroring the main project structure:

```
tests/
├── CMakeLists.txt          # Test configuration
├── core/                   # Core component tests
│   └── test_ray.cpp
└── world/                  # World logic tests
    ├── test_chunk.cpp
    ├── test_block_registry.cpp
    └── test_voxel_raycast.cpp
```

## Writing a New Test

### 1. Create a Test File

Create a new `.cpp` file in the appropriate directory (e.g., `tests/world/test_myfeature.cpp`):

```cpp
#include <gtest/gtest.h>
#include "world/MyFeature.h"

// Test fixture (optional, for shared setup/teardown)
class MyFeatureTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test data
    }
    
    void TearDown() override {
        // Clean up after tests
    }
    
    // Shared test data
    MyFeature feature;
};

// Basic test
TEST(MyFeatureTest, BasicFunctionality) {
    MyFeature feature;
    EXPECT_EQ(feature.Calculate(5), 25);
}

// Test using fixture
TEST_F(MyFeatureTest, AdvancedFunctionality) {
    feature.SetValue(10);
    EXPECT_TRUE(feature.IsValid());
}
```

### 2. Add Test to CMakeLists.txt

Edit `tests/CMakeLists.txt` and add your test file to `TEST_SOURCES`:

```cmake
set(TEST_SOURCES
    # Core tests
    core/test_ray.cpp
    
    # World tests
    world/test_chunk.cpp
    world/test_block_registry.cpp
    world/test_voxel_raycast.cpp
    world/test_myfeature.cpp  # Add your test here
)
```

If your test requires additional source files from the main project, add them to `PROJECT_TEST_SOURCES`:

```cmake
set(PROJECT_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/world/Chunk.cpp
    ${CMAKE_SOURCE_DIR}/world/MyFeature.cpp  # Add required source
    # ... other sources
)
```

### 3. Build and Run Tests

```bash
# Build tests
cmake --build build --target VoxelTests

# Run all tests
./build/tests/VoxelTests

# Run only your tests
./build/tests/VoxelTests --gtest_filter=MyFeatureTest.*

# Run with verbose output
./build/tests/VoxelTests --gtest_filter=MyFeatureTest.* --gtest_color=yes
```

## GTest Assertions

Common assertions you'll use:

### Boolean Checks
```cpp
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);
```

### Value Comparisons
```cpp
EXPECT_EQ(actual, expected);  // Equal
EXPECT_NE(actual, expected);  // Not equal
EXPECT_LT(a, b);              // Less than
EXPECT_LE(a, b);              // Less than or equal
EXPECT_GT(a, b);              // Greater than
EXPECT_GE(a, b);              // Greater than or equal
```

### Floating Point
```cpp
EXPECT_FLOAT_EQ(actual, expected);
EXPECT_NEAR(actual, expected, tolerance);
```

### String Comparisons
```cpp
EXPECT_STREQ(str1, str2);    // C-string equal
EXPECT_EQ(str1, str2);       // std::string equal
```

## Best Practices

### 1. Test One Thing at a Time
Each test should focus on a single behavior:

```cpp
// Good
TEST(ChunkTest, SetBlock) {
    Chunk chunk;
    chunk.SetBlock(5, 64, 7, BlockType::Stone);
    EXPECT_EQ(chunk.GetBlock(5, 64, 7), BlockType::Stone);
}

// Bad - testing multiple things
TEST(ChunkTest, Everything) {
    Chunk chunk;
    chunk.SetBlock(5, 64, 7, BlockType::Stone);
    EXPECT_EQ(chunk.GetBlock(5, 64, 7), BlockType::Stone);
    chunk.RebuildHeightmap();
    EXPECT_EQ(chunk.GetHeightAt(5, 7), 64);
}
```

### 2. Use Descriptive Test Names
Test names should clearly describe what they're testing:

```cpp
// Good
TEST(ChunkTest, GetBlockReturnsAirForUnsetPositions)
TEST(ChunkTest, SetBlockUpdatesSpecificPosition)
TEST(ChunkTest, IsInBoundsReturnsFalseForNegativeCoordinates)

// Bad
TEST(ChunkTest, Test1)
TEST(ChunkTest, TestChunk)
TEST(ChunkTest, BasicTest)
```

### 3. Avoid OpenGL/GLFW Dependencies
Unit tests run without a graphics context. Test only pure C++ logic:

```cpp
// Good - tests logic without OpenGL
TEST(ChunkTest, GetBlock) {
    Chunk chunk;
    BlockType block = chunk.GetBlock(0, 0, 0);
    EXPECT_EQ(block, BlockType::Air);
}

// Bad - requires OpenGL context
TEST(ChunkTest, Render) {
    Chunk chunk;
    chunk.BuildMesh();    // This calls OpenGL functions
    chunk.Render();       // Will fail without graphics context
}
```

### 4. Use Test Fixtures for Shared Setup
When multiple tests need the same setup, use a fixture:

```cpp
class ChunkManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = new ChunkManager();
    }
    
    void TearDown() override {
        delete manager;
    }
    
    ChunkManager* manager;
};

TEST_F(ChunkManagerTest, LoadsChunksAroundPlayer) {
    manager->Update(glm::vec3(0, 64, 0));
    EXPECT_GT(manager->GetLoadedChunkCount(), 0);
}
```

### 5. Test Edge Cases
Always test boundary conditions:

```cpp
TEST(ChunkTest, BoundaryConditions) {
    Chunk chunk;
    
    // Test minimum bounds
    EXPECT_TRUE(chunk.IsInBounds(0, 0, 0));
    EXPECT_FALSE(chunk.IsInBounds(-1, 0, 0));
    
    // Test maximum bounds
    EXPECT_TRUE(chunk.IsInBounds(15, 255, 15));
    EXPECT_FALSE(chunk.IsInBounds(16, 256, 16));
}
```

## Running Tests in CI

Tests are automatically run on GitHub Actions for every push and pull request. The workflow is defined in `.github/workflows/tests.yml`.

To ensure your tests pass in CI:
1. Make sure they don't require graphics context
2. Avoid hardcoded paths (use relative paths)
3. Clean up temporary files in test teardown
4. Ensure tests are deterministic (no random behavior)

## Debugging Failing Tests

### Run with verbose output
```bash
./build/tests/VoxelTests --gtest_filter=MyFailingTest.* --gtest_color=yes
```

### Run with GDB
```bash
gdb --args ./build/tests/VoxelTests --gtest_filter=MyFailingTest.*
```

### Check test output
```bash
./build/tests/VoxelTests --gtest_output=xml:test_results.xml
cat test_results.xml
```

## Resources

- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Google Test Advanced Guide](https://google.github.io/googletest/advanced.html)
- [Google Test FAQ](https://google.github.io/googletest/faq.html)
