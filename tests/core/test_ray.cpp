#include <gtest/gtest.h>
#include "core/scene/Ray.h"
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

using namespace Core;

/**
 * Test fixture for Ray tests
 */
class RayTest : public ::testing::Test {
protected:
    const float EPSILON = 0.0001f;
    
    bool Vec3Equal(const glm::vec3& a, const glm::vec3& b, float epsilon = 0.0001f) {
        return glm::all(glm::epsilonEqual(a, b, epsilon));
    }
};

/**
 * Test default Ray constructor
 */
TEST_F(RayTest, DefaultConstructor) {
    Ray ray;
    
    EXPECT_TRUE(Vec3Equal(ray.origin, glm::vec3(0.0f)));
    EXPECT_TRUE(Vec3Equal(ray.direction, glm::vec3(0.0f, 0.0f, -1.0f)));
}

/**
 * Test Ray constructor with parameters
 */
TEST_F(RayTest, ParameterizedConstructor) {
    glm::vec3 origin(1.0f, 2.0f, 3.0f);
    glm::vec3 direction(1.0f, 0.0f, 0.0f);
    
    Ray ray(origin, direction);
    
    EXPECT_TRUE(Vec3Equal(ray.origin, origin));
    EXPECT_TRUE(Vec3Equal(ray.direction, glm::normalize(direction)));
}

/**
 * Test that Ray constructor normalizes direction
 */
TEST_F(RayTest, DirectionNormalization) {
    glm::vec3 origin(0.0f);
    glm::vec3 unnormalizedDirection(5.0f, 0.0f, 0.0f);
    
    Ray ray(origin, unnormalizedDirection);
    
    // Direction should be normalized to unit length
    EXPECT_NEAR(glm::length(ray.direction), 1.0f, EPSILON);
    EXPECT_TRUE(Vec3Equal(ray.direction, glm::vec3(1.0f, 0.0f, 0.0f)));
}

/**
 * Test GetPoint method with various distances
 */
TEST_F(RayTest, GetPoint) {
    Ray ray(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    
    // Point at origin (t=0)
    glm::vec3 p0 = ray.GetPoint(0.0f);
    EXPECT_TRUE(Vec3Equal(p0, glm::vec3(0.0f, 0.0f, 0.0f)));
    
    // Point at distance 1
    glm::vec3 p1 = ray.GetPoint(1.0f);
    EXPECT_TRUE(Vec3Equal(p1, glm::vec3(1.0f, 0.0f, 0.0f)));
    
    // Point at distance 5
    glm::vec3 p5 = ray.GetPoint(5.0f);
    EXPECT_TRUE(Vec3Equal(p5, glm::vec3(5.0f, 0.0f, 0.0f)));
}

/**
 * Test GetPoint with diagonal ray
 */
TEST_F(RayTest, GetPointDiagonal) {
    glm::vec3 origin(1.0f, 1.0f, 1.0f);
    glm::vec3 direction = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
    
    Ray ray(origin, direction);
    
    // At distance sqrt(3), we should move 1 unit in each direction
    float distance = std::sqrt(3.0f);
    glm::vec3 point = ray.GetPoint(distance);
    
    EXPECT_TRUE(Vec3Equal(point, glm::vec3(2.0f, 2.0f, 2.0f), 0.001f));
}

/**
 * Test negative distance (point behind ray origin)
 */
TEST_F(RayTest, GetPointNegativeDistance) {
    Ray ray(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    
    glm::vec3 point = ray.GetPoint(-2.0f);
    EXPECT_TRUE(Vec3Equal(point, glm::vec3(3.0f, 0.0f, 0.0f)));
}
