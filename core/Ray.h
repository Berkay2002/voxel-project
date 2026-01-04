#pragma once

#include <glm/glm.hpp>

namespace Core {

/**
 * Simple ray structure for raycasting operations.
 * Used for voxel picking, collision detection, etc.
 */
struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;  // Should be normalized

    Ray() : origin(0.0f), direction(0.0f, 0.0f, -1.0f) {}
    
    Ray(const glm::vec3& o, const glm::vec3& d)
        : origin(o), direction(glm::normalize(d)) {}

    /**
     * Get a point along the ray at distance t from origin.
     * @param t Distance along the ray
     * @return Point at origin + direction * t
     */
    [[nodiscard]] glm::vec3 GetPoint(float t) const {
        return origin + direction * t;
    }
};

} // namespace Core
