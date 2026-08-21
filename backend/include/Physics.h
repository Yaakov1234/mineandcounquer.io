#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <optional>
#include "common/Types.h"

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    AABB() = default;
    AABB(const glm::vec3& center, const glm::vec3& halfExtents)
        : min(center - halfExtents), max(center + halfExtents) {}

    bool intersects(const AABB& other) const;
    bool contains(const glm::vec3& point) const;
    float distance(const glm::vec3& point) const;
};

struct RaycastHit {
    glm::vec3 point;
    float distance;
    bool hit = false;
};

class Physics {
public:
    Physics();
    ~Physics() = default;

    // Collision detection
    bool checkCollision(const AABB& a, const AABB& b) const;
    bool checkCollisionWithPoint(const AABB& aabb, const glm::vec3& point) const;
    
    // Raycast - for weapon hit detection
    RaycastHit raycast(const glm::vec3& start, const glm::vec3& end) const;
    bool canSeeTarget(const glm::vec3& shooterPos, const glm::vec3& targetPos) const;

    // Spatial queries
    std::vector<AABB> getNearbyAABBs(const AABB& area) const;
    std::vector<glm::vec3> getAvailableSpawnPoints(const glm::vec3& center, float radius, uint32_t count) const;

    // Validation
    bool isValidPosition(const glm::vec3& pos, const glm::vec3& worldMin, const glm::vec3& worldMax) const;
    bool isOutOfBounds(const glm::vec3& pos, float worldRadius) const;
    
    // Distance calculations
    float distance(const glm::vec3& a, const glm::vec3& b) const;
};
