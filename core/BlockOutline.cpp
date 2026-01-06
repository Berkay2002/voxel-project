#include "BlockOutline.h"
#include "graphics/Shader.h"
#include "Logger.h"
#include <glad/gl.h>

namespace Core {

BlockOutline::BlockOutline() = default;

BlockOutline::~BlockOutline() {
    Cleanup();
}

void BlockOutline::Setup() {
    // Define the 12 edges of a unit cube as line segments
    // Each edge is 2 vertices, so 24 vertices total for GL_LINES
    // Cube spans from (0,0,0) to (1,1,1) to match block coordinates
    float vertices[] = {
        // Bottom face edges (y = 0)
        0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Edge 1: (0,0,0) -> (1,0,0)
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 1.0f,  // Edge 2: (1,0,0) -> (1,0,1)
        1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  // Edge 3: (1,0,1) -> (0,0,1)
        0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,  // Edge 4: (0,0,1) -> (0,0,0)
        
        // Top face edges (y = 1)
        0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,  // Edge 5: (0,1,0) -> (1,1,0)
        1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  // Edge 6: (1,1,0) -> (1,1,1)
        1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,  // Edge 7: (1,1,1) -> (0,1,1)
        0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // Edge 8: (0,1,1) -> (0,1,0)
        
        // Vertical edges connecting top and bottom
        0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Edge 9:  (0,0,0) -> (0,1,0)
        1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,  // Edge 10: (1,0,0) -> (1,1,0)
        1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,  // Edge 11: (1,0,1) -> (1,1,1)
        0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 1.0f   // Edge 12: (0,0,1) -> (0,1,1)
    };
    
    m_VertexCount = 24;  // 12 edges * 2 vertices per edge
    
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    
    glBindVertexArray(m_VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Position attribute (location 0) - vec3
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    
    LOG_INFO("BlockOutline initialized with " + std::to_string(m_VertexCount) + " vertices");
}

void BlockOutline::Render(const glm::ivec3& blockPos, Shader& shader, const glm::mat4& viewProj) {
    if (m_VAO == 0) {
        return;  // Not initialized
    }
    
    shader.Bind();
    
    // Set uniforms
    shader.SetMat4("u_MVP", viewProj);
    shader.SetVec3("u_BlockPos", glm::vec3(blockPos));
    shader.SetFloat("u_Scale", m_Scale);
    shader.SetVec4("u_OutlineColor", m_Color);
    
    // Set line width (may not work on all drivers, but worth trying)
    glLineWidth(m_LineWidth);
    
    // Disable depth write so outline is always visible but still uses depth test
    // This makes the outline appear "on top" but still occluded by closer geometry
    glDepthMask(GL_FALSE);
    
    // Enable blending for transparent outline
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_LINES, 0, m_VertexCount);
    glBindVertexArray(0);
    
    // Restore state
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    
    shader.Unbind();
}

void BlockOutline::Cleanup() {
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    if (m_VBO != 0) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
}

} // namespace Core
