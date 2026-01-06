#pragma once

#include <glm/glm.hpp>
#include <array>

namespace Core {

/**
 * Frustum class for view frustum culling.
 * Extracts 6 planes from the view-projection matrix and tests AABBs against them.
 */
class Frustum {
public:
    Frustum() = default;

    /**
     * Extract the 6 frustum planes from a view-projection matrix.
     * Call this once per frame after camera/projection changes.
     */
    void ExtractPlanes(const glm::mat4& viewProj);

    /**
     * Test if an axis-aligned bounding box is visible in the frustum.
     * @param min The minimum corner of the AABB
     * @param max The maximum corner of the AABB
     * @return true if any part of the AABB is potentially visible
     */
    [[nodiscard]] bool IsAABBVisible(const glm::vec3& min, const glm::vec3& max) const;

private:
    struct Plane {
        glm::vec3 normal{0.0f};
        float distance = 0.0f;

        // Signed distance from plane to point (positive = in front, negative = behind)
        [[nodiscard]] float DistanceToPoint(const glm::vec3& point) const {
            return glm::dot(normal, point) + distance;
        }
    };

    // Frustum planes: Left, Right, Bottom, Top, Near, Far
    std::array<Plane, 6> m_Planes;
};

} // namespace Core
