#include "Physics.h"
#include <cmath>
#include <random>
#include <glm/gtc/random.hpp>

bool AABB::intersects(const AABB& other) const {
    return (min.x <= other.max.x && max.x >= other.min.x) &&
           (min.y <= other.max.y && max.y >= other.min.y) &&
           (min.z <= other.max.z && max.z >= other.min.z);
}

bool AABB::contains(const glm::vec3& point) const {
    return (point.x >= min.x && point.x <= max.x) &&
           (point.y >= min.y && point.y <= max.y) &&
           (point.z >= min.z && point.z <= max.z);
}

float AABB::distance(const glm::vec3& point) const {
    glm::vec3 closest;
    closest.x = glm::clamp(point.x, min.x, max.x);
    closest.y = glm::clamp(point.y, min.y, max.y);
    closest.z = glm::clamp(point.z, min.z, max.z);
    return glm::distance(point, closest);
}

Physics::Physics() = default;

bool Physics::checkCollision(const AABB& a, const AABB& b) const {
    return a.intersects(b);
}

bool Physics::checkCollisionWithPoint(const AABB& aabb, const glm::vec3& point) const {
    return aabb.contains(point);
}

RaycastHit Physics::raycast(const glm::vec3& start, const glm::vec3& end) const {
    RaycastHit hit;
    hit.point = end;
    hit.distance = glm::distance(start, end);
    hit.hit = true;  // For now, assume ray always hits
    return hit;
}

bool Physics::canSeeTarget(const glm::vec3& shooterPos, const glm::vec3& targetPos) const {
    // Line of sight check - for now, simple distance check
    // TODO: Implement actual raycasting against obstacles
    return glm::distance(shooterPos, targetPos) < 1000.0f;
}

std::vector<AABB> Physics::getNearbyAABBs(const AABB& area) const {
    // TODO: Implement spatial partitioning query
    return std::vector<AABB>();
}

std::vector<glm::vec3> Physics::getAvailableSpawnPoints(const glm::vec3& center, float radius, uint32_t count) const {
    std::vector<glm::vec3> spawnPoints;
    for (uint32_t i = 0; i < count; ++i) {
        glm::vec3 randomOffset = glm::ballRand(radius);
        spawnPoints.push_back(center + randomOffset);
    }
    return spawnPoints;
}

bool Physics::isValidPosition(const glm::vec3& pos, const glm::vec3& worldMin, const glm::vec3& worldMax) const {
    return pos.x >= worldMin.x && pos.x <= worldMax.x &&
           pos.y >= worldMin.y && pos.y <= worldMax.y &&
           pos.z >= worldMin.z && pos.z <= worldMax.z;
}

bool Physics::isOutOfBounds(const glm::vec3& pos, float worldRadius) const {
    return glm::length(pos) > worldRadius;
}

float Physics::distance(const glm::vec3& a, const glm::vec3& b) const {
    return glm::distance(a, b);
}
