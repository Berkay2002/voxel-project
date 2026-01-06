#pragma once

#include <glm/glm.hpp>

namespace Core {

class Shader;

/**
 * SelectionRenderer - Renders a wireframe cube around a targeted block
 * 
 * This renders a slightly scaled-up wireframe cube at the given block position
 * to provide visual feedback for which block the player is targeting.
 * 
 * Renamed from BlockOutline to match AAA engine conventions.
 */
class SelectionRenderer {
public:
    SelectionRenderer();
    ~SelectionRenderer();

    // Non-copyable
    SelectionRenderer(const SelectionRenderer&) = delete;
    SelectionRenderer& operator=(const SelectionRenderer&) = delete;

    // Initialize OpenGL resources (call once after context is created)
    void Setup();

    // Render the outline at the given block position
    // viewProj: combined view-projection matrix from camera
    void Render(const glm::ivec3& blockPos, Shader& shader, const glm::mat4& viewProj);

    // Cleanup OpenGL resources
    void Cleanup();

    // Configuration
    void SetScale(float scale) { m_Scale = scale; }
    void SetColor(const glm::vec4& color) { m_Color = color; }
    void SetLineWidth(float width) { m_LineWidth = width; }

private:
    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
    unsigned int m_VertexCount = 0;

    // Visual settings
    float m_Scale = 1.005f;                          // Slightly larger than 1.0 to sit outside block
    glm::vec4 m_Color = glm::vec4(0.0f, 0.0f, 0.0f, 0.8f);  // Black with slight transparency
    float m_LineWidth = 2.0f;
};

} // namespace Core
