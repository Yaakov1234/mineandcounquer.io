#include "BuildingSystem.h"
#include <map>
#include <iostream>

BuildingSystem::BuildingSystem() {
    initializeConfigs();
}

bool BuildingSystem::canPlaceStructure(const glm::vec3& position, StructureType type) const {
    // Basic validation - can be expanded
    return true;
}

bool BuildingSystem::checkPlacementCollision(const glm::vec3& pos, float radius, const std::vector<std::unique_ptr<Structure>>& existing) const {
    for (const auto& structure : existing) {
        if (glm::distance(pos, structure->position) < radius + getTerritoryRadius(structure->type)) {
            return true;  // Collision detected
        }
    }
    return false;
}

uint32_t BuildingSystem::getGemCost(StructureType type) const {
    auto it = structureConfigs_.find(type);
    return it != structureConfigs_.end() ? it->second.gemCost : 0;
}

uint32_t BuildingSystem::getRockCost(StructureType type) const {
    auto it = structureConfigs_.find(type);
    return it != structureConfigs_.end() ? it->second.rockCost : 0;
}

float BuildingSystem::getStructureHealth(StructureType type) const {
    auto it = structureConfigs_.find(type);
    return it != structureConfigs_.end() ? it->second.health : 100.0f;
}

uint64_t BuildingSystem::getBuildTimeMs(StructureType type) const {
    auto it = structureConfigs_.find(type);
    return it != structureConfigs_.end() ? it->second.buildTimeMs : 5000;  // Default 5 seconds
}

uint64_t BuildingSystem::calculateTerritorySize(StructureType type) const {
    auto it = structureConfigs_.find(type);
    if (it != structureConfigs_.end()) {
        float radius = it->second.radius;
        return static_cast<uint64_t>(radius * radius * 3.14159f);
    }
    return 0;
}

float BuildingSystem::getTerritoryRadius(StructureType type) const {
    auto it = structureConfigs_.find(type);
    return it != structureConfigs_.end() ? it->second.radius : 10.0f;
}

std::string BuildingSystem::getStructureName(StructureType type) const {
    static const std::map<StructureType, std::string> names = {
        {StructureType::WALL, "Wall"},
        {StructureType::TOWER, "Tower"},
        {StructureType::BASE, "Base"},
        {StructureType::STORAGE, "Storage"},
        {StructureType::TURRET, "Turret"}
    };
    auto it = names.find(type);
    return it != names.end() ? it->second : "Unknown";
}

float BuildingSystem::getStructureRadius(StructureType type) const {
    auto it = structureConfigs_.find(type);
    return it != structureConfigs_.end() ? it->second.radius : 10.0f;
}

uint32_t BuildingSystem::getUpgradeGemCost(StructureType type) const {
    return getGemCost(type) / 2;  // Upgrade costs half of build cost
}

float BuildingSystem::getUpgradeHealthBonus(StructureType type) const {
    return getStructureHealth(type) * 0.25f;  // 25% health increase
}

void BuildingSystem::initializeConfigs() {
    structureConfigs_[StructureType::WALL] = {10, 20, 50.0f, 10.0f, 3000};
    structureConfigs_[StructureType::TOWER] = {30, 40, 100.0f, 15.0f, 5000};
    structureConfigs_[StructureType::BASE] = {50, 100, 200.0f, 30.0f, 10000};
    structureConfigs_[StructureType::STORAGE] = {20, 50, 75.0f, 12.0f, 4000};
    structureConfigs_[StructureType::TURRET] = {40, 30, 80.0f, 8.0f, 6000};
}
