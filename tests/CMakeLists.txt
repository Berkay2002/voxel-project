# ============================================================================
# Test Executable
# ============================================================================

# Shared test sources (non-OpenGL components only)
set(TEST_SOURCES
    # Core tests
    core/test_ray.cpp
    
    # World tests
    world/test_chunk.cpp
    world/test_block_registry.cpp
    world/test_voxel_raycast.cpp
)

# Test-specific source files from main project (no OpenGL dependencies)
set(PROJECT_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/world/Chunk.cpp
    ${CMAKE_SOURCE_DIR}/world/BlockRegistry.cpp
    ${CMAKE_SOURCE_DIR}/world/VoxelRaycast.cpp
    ${CMAKE_SOURCE_DIR}/world/ChunkManager.cpp
    ${CMAKE_SOURCE_DIR}/world/ChunkMeshBuilder.cpp
    ${CMAKE_SOURCE_DIR}/world/TerrainGenerator.cpp
    ${CMAKE_SOURCE_DIR}/world/SpaghettiCaveCarver.cpp
    ${CMAKE_SOURCE_DIR}/world/GameConfig.cpp
    ${CMAKE_SOURCE_DIR}/core/rendering/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/core/graphics/TextureArray.cpp
    ${CMAKE_SOURCE_DIR}/core/graphics/Texture.cpp
    ${CMAKE_SOURCE_DIR}/core/graphics/Shader.cpp
    ${CMAKE_SOURCE_DIR}/core/graphics/VertexArray.cpp
    ${CMAKE_SOURCE_DIR}/core/graphics/VertexBuffer.cpp
    ${CMAKE_SOURCE_DIR}/core/graphics/IndexBuffer.cpp
    ${CMAKE_SOURCE_DIR}/core/scene/Frustum.cpp
    ${CMAKE_SOURCE_DIR}/core/scene/Camera.cpp
)

add_executable(VoxelTests ${TEST_SOURCES} ${PROJECT_TEST_SOURCES})

target_include_directories(VoxelTests PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${stb_SOURCE_DIR}
    ${fastnoiselite_SOURCE_DIR}/Cpp
    ${thread_pool_SOURCE_DIR}/include
)

target_link_libraries(VoxelTests PRIVATE
    GTest::gtest_main
    glm::glm
    nlohmann_json::nlohmann_json
    glad_gl_core_46
)

# ============================================================================
# Test Discovery
# ============================================================================
include(GoogleTest)
gtest_discover_tests(VoxelTests)

# ============================================================================
# Copy test assets to build directory
# ============================================================================
add_custom_command(TARGET VoxelTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:VoxelTests>/assets
    COMMENT "Copying assets to test directory"
)
