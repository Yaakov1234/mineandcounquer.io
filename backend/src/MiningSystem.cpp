#include "MiningSystem.h"
#include <glm/gtc/random.hpp>

MiningSystem::MiningSystem() 
    : rng_(std::random_device()()) {
}

Resource MiningSystem::generateResource(ResourceID id, const glm::vec3& position) {
    std::uniform_int_distribution<> rarityDis(gemMinRarity_, gemMaxRarity_);

    Resource resource;
    resource.id = id;
    resource.position = position;
    resource.collected = false;

    if (rarityDis(rng_) % 2 == 0) {
        resource.type = ResourceType::GEM;
        resource.value = rarityDis(rng_);
    } else {
        resource.type = ResourceType::ROCK;
        resource.value = std::uniform_int_distribution<>(rockMinAmount_, rockMaxAmount_)(rng_);
    }

    return resource;
}

std::vector<glm::vec3> MiningSystem::getRandomMinePositions(uint32_t count, const glm::vec3& center, float radius) const {
    std::vector<glm::vec3> positions;
    for (uint32_t i = 0; i < count; ++i) {
        glm::vec3 randomOffset = glm::ballRand(radius);
        positions.push_back(center + randomOffset);
    }
    return positions;
}

std::vector<glm::vec3> MiningSystem::getResourceClusters(const glm::vec3& center, float radius, uint32_t clusterCount) const {
    std::vector<glm::vec3> clusters = getRandomMinePositions(clusterCount, center, radius);
    // Resources are already clustered by their spawn positions
    return clusters;
}

uint32_t MiningSystem::calculateGemValue(uint32_t rarity) const {
    // Value increases exponentially with rarity
    return rarity * rarity * 10;
}

uint32_t MiningSystem::getGemRarity() const {
    std::uniform_int_distribution<> dis(gemMinRarity_, gemMaxRarity_);
    return dis(rng_);
}

uint32_t MiningSystem::getRockAmount() const {
    std::uniform_int_distribution<> dis(rockMinAmount_, rockMaxAmount_);
    return dis(rng_);
}

uint32_t MiningSystem::getResourceValue(const Resource& resource) const {
    if (resource.type == ResourceType::GEM) {
        return calculateGemValue(resource.value);
    } else {
        return resource.value * rockValue_;
    }
}
