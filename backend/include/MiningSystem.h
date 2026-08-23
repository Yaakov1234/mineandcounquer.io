#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <random>
#include "common/Types.h"

class MiningSystem {
public:
    MiningSystem();
    ~MiningSystem() = default;

    // Resource generation
    Resource generateResource(ResourceID id, const glm::vec3& position);
    std::vector<glm::vec3> getRandomMinePositions(uint32_t count, const glm::vec3& center, float radius) const;
    std::vector<glm::vec3> getResourceClusters(const glm::vec3& center, float radius, uint32_t clusterCount) const;

    // Mining mechanics
    uint32_t calculateGemValue(uint32_t rarity) const;
    uint32_t getRockValue() const { return rockValue_; }
    uint32_t getGemRarity() const;
    uint32_t getRockAmount() const;
    
    // Resource properties
    bool isGem(const Resource& resource) const { return resource.type == ResourceType::GEM; }
    bool isRock(const Resource& resource) const { return resource.type == ResourceType::ROCK; }
    uint32_t getResourceValue(const Resource& resource) const;

private:
    mutable std::mt19937 rng_;
    uint32_t rockValue_ = 1;
    uint32_t gemMinRarity_ = 1;
    uint32_t gemMaxRarity_ = 5;
    uint32_t rockMinAmount_ = 1;
    uint32_t rockMaxAmount_ = 10;
};

