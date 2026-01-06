#include "Frustum.h"

namespace Core {

void Frustum::ExtractPlanes(const glm::mat4& viewProj) {
    // Extract frustum planes from the view-projection matrix
    // Using the Gribb-Hartmann method (row extraction)
    // Each plane: ax + by + cz + d = 0
    // Stored as: normal = (a, b, c), distance = d

    // Left plane: row 3 + row 0
    m_Planes[0].normal.x = viewProj[0][3] + viewProj[0][0];
    m_Planes[0].normal.y = viewProj[1][3] + viewProj[1][0];
    m_Planes[0].normal.z = viewProj[2][3] + viewProj[2][0];
    m_Planes[0].distance = viewProj[3][3] + viewProj[3][0];

    // Right plane: row 3 - row 0
    m_Planes[1].normal.x = viewProj[0][3] - viewProj[0][0];
    m_Planes[1].normal.y = viewProj[1][3] - viewProj[1][0];
    m_Planes[1].normal.z = viewProj[2][3] - viewProj[2][0];
    m_Planes[1].distance = viewProj[3][3] - viewProj[3][0];

    // Bottom plane: row 3 + row 1
    m_Planes[2].normal.x = viewProj[0][3] + viewProj[0][1];
    m_Planes[2].normal.y = viewProj[1][3] + viewProj[1][1];
    m_Planes[2].normal.z = viewProj[2][3] + viewProj[2][1];
    m_Planes[2].distance = viewProj[3][3] + viewProj[3][1];

    // Top plane: row 3 - row 1
    m_Planes[3].normal.x = viewProj[0][3] - viewProj[0][1];
    m_Planes[3].normal.y = viewProj[1][3] - viewProj[1][1];
    m_Planes[3].normal.z = viewProj[2][3] - viewProj[2][1];
    m_Planes[3].distance = viewProj[3][3] - viewProj[3][1];

    // Near plane: row 3 + row 2
    m_Planes[4].normal.x = viewProj[0][3] + viewProj[0][2];
    m_Planes[4].normal.y = viewProj[1][3] + viewProj[1][2];
    m_Planes[4].normal.z = viewProj[2][3] + viewProj[2][2];
    m_Planes[4].distance = viewProj[3][3] + viewProj[3][2];

    // Far plane: row 3 - row 2
    m_Planes[5].normal.x = viewProj[0][3] - viewProj[0][2];
    m_Planes[5].normal.y = viewProj[1][3] - viewProj[1][2];
    m_Planes[5].normal.z = viewProj[2][3] - viewProj[2][2];
    m_Planes[5].distance = viewProj[3][3] - viewProj[3][2];

    // Normalize all planes
    for (auto& plane : m_Planes) {
        float length = glm::length(plane.normal);
        if (length > 0.0f) {
            plane.normal /= length;
            plane.distance /= length;
        }
    }
}

bool Frustum::IsAABBVisible(const glm::vec3& min, const glm::vec3& max) const {
    // Test AABB against all 6 frustum planes
    // For each plane, find the AABB vertex most in the direction of the plane normal (positive vertex)
    // If the positive vertex is behind the plane, the AABB is completely outside

    for (const auto& plane : m_Planes) {
        // Find the "positive vertex" (the corner most in the direction of the plane normal)
        glm::vec3 positiveVertex;
        positiveVertex.x = (plane.normal.x >= 0.0f) ? max.x : min.x;
        positiveVertex.y = (plane.normal.y >= 0.0f) ? max.y : min.y;
        positiveVertex.z = (plane.normal.z >= 0.0f) ? max.z : min.z;

        // If the positive vertex is behind this plane, the AABB is completely outside
        if (plane.DistanceToPoint(positiveVertex) < 0.0f) {
            return false;
        }
    }

    // The AABB is at least partially inside all planes
    return true;
}

} // namespace Core
