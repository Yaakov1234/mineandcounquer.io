#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <map>
#include "common/Types.h"

class BuildingSystem {
public:
    BuildingSystem();
    ~BuildingSystem() = default;

    // Structure placement validation
    bool canPlaceStructure(const glm::vec3& position, StructureType type) const;
    bool checkPlacementCollision(const glm::vec3& pos, float radius, const std::vector<std::unique_ptr<Structure>>& existing) const;
    
    // Cost & health
    uint32_t getGemCost(StructureType type) const;
    uint32_t getRockCost(StructureType type) const;
    float getStructureHealth(StructureType type) const;
    uint64_t getBuildTimeMs(StructureType type) const;

    // Territory management
    uint64_t calculateTerritorySize(StructureType type) const;
    float getTerritoryRadius(StructureType type) const;
    
    // Structure info
    std::string getStructureName(StructureType type) const;
    float getStructureRadius(StructureType type) const;

    // Upgrade system
    uint32_t getUpgradeGemCost(StructureType type) const;
    float getUpgradeHealthBonus(StructureType type) const;

private:
    std::map<StructureType, StructureConfig> structureConfigs_;
    void initializeConfigs();
};
