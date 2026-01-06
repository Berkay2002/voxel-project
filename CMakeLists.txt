cmake_minimum_required(VERSION 3.28)
project(VoxelEngine LANGUAGES C CXX)

# ============================================================================
# C++ Standard
# ============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# Compiler Warnings
# ============================================================================
if(MSVC)
    add_compile_options(/W4 /permissive-)
    # Prevent Windows.h from defining min/max macros that conflict with STL
    add_compile_definitions(NOMINMAX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ============================================================================
# Dependencies via FetchContent
# ============================================================================
include(FetchContent)

# GLFW 3.4
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM 1.0.1 (latest stable)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

# GLAD (OpenGL 4.6 Core Profile)
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.8
    SOURCE_SUBDIR  cmake
)
FetchContent_MakeAvailable(glad)
glad_add_library(glad_gl_core_46 REPRODUCIBLE API gl:core=4.6)

# stb_image (header-only image loading)
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

# FastNoiseLite (header-only noise library)
FetchContent_Declare(
    FastNoiseLite
    GIT_REPOSITORY https://github.com/Auburn/FastNoiseLite.git
    GIT_TAG        v1.1.1
)
FetchContent_MakeAvailable(FastNoiseLite)

# BS::thread_pool (header-only thread pool library for async chunk loading)
FetchContent_Declare(
    thread_pool
    GIT_REPOSITORY https://github.com/bshoshany/thread-pool.git
    GIT_TAG        v5.0.0
)
FetchContent_MakeAvailable(thread_pool)

# nlohmann/json (JSON parsing for data-driven configuration)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(json)


# ============================================================================
# Executable
# ============================================================================
add_executable(${PROJECT_NAME}
    main.cpp
    core/Application.cpp
    core/graphics/Shader.cpp
    core/graphics/VertexBuffer.cpp
    core/graphics/IndexBuffer.cpp
    core/graphics/VertexArray.cpp
    core/graphics/Texture.cpp
    core/graphics/TextureArray.cpp
    core/scene/Camera.cpp
    core/scene/Frustum.cpp
    core/rendering/SelectionRenderer.cpp
    core/rendering/ShadowMap.cpp
    core/rendering/SSAO.cpp
    core/rendering/TextureManager.cpp
    core/window/Window.cpp
    core/settings/VideoSettings.cpp
    core/settings/GraphicsSettings.cpp
    core/atmosphere/SkySystem.cpp
    world/Chunk.cpp
    world/ChunkMeshBuilder.cpp
    world/TerrainGenerator.cpp
    world/ChunkManager.cpp
    world/SpaghettiCaveCarver.cpp
    world/BlockRegistry.cpp
    world/VoxelRaycast.cpp
    world/GameConfig.cpp
    ui/UIRenderer.cpp
    ui/TitleScreen.cpp
    ui/LoadingScreen.cpp
    ui/SettingsScreen.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${stb_SOURCE_DIR}
    ${fastnoiselite_SOURCE_DIR}/Cpp
    ${thread_pool_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glm::glm
    glad_gl_core_46
    nlohmann_json::nlohmann_json
)

# ============================================================================
# Copy assets to build directory
# ============================================================================
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Copying assets to build directory"
)

