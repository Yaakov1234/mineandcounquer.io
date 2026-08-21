#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "common/Types.h"
#include "Player.h"
#include "Physics.h"
#include "MiningSystem.h"
#include "BuildingSystem.h"
#include "CombatSystem.h"

class World {
public:
    World(WorldID id);
    ~World();

    // Lifecycle
    void update(float deltaTime);

    // Player management
    PlayerID addPlayer(const std::string& username);
    void removePlayer(PlayerID id);
    Player* getPlayer(PlayerID id);
    const std::vector<Player*>& getAllPlayers() const { return players_; }
    size_t getPlayerCount() const { return players_.size(); }

    // Structure management
    StructureID addStructure(PlayerID ownerId, const glm::vec3& pos, StructureType type);
    void removeStructure(StructureID id);
    Structure* getStructure(StructureID id);
    const std::vector<std::unique_ptr<Structure>>& getAllStructures() const { return structures_; }
    std::vector<StructureID> getPlayerStructures(PlayerID ownerId) const;
    uint64_t getPlayerTerritory(PlayerID ownerId) const;

    // Resource management
    ResourceID addResource(ResourceType type, const glm::vec3& pos, uint32_t value);
    void removeResource(ResourceID id);
    Resource* getResource(ResourceID id);
    const std::vector<std::unique_ptr<Resource>>& getAllResources() const { return resources_; }
    std::vector<ResourceID> getNearbyResources(const glm::vec3& pos, float radius) const;

    // Zone transitions
    bool canExitMine(PlayerID playerId) const;
    void movePlayerToSurface(PlayerID playerId);
    void movePlayerToMine(PlayerID playerId);

    // Combat actions
    bool fireWeapon(PlayerID shooterId, const glm::vec3& targetPos, WeaponType weapon);
    void damagePlayer(PlayerID targetId, float damage);
    void damageStructure(StructureID structureId, float damage);
    void killPlayer(PlayerID playerId, PlayerID killerId);

    // Spawn logic
    glm::vec3 getRandomMineSpawn() const;
    glm::vec3 getRandomSurfaceSpawn() const;

    // Accessors
    WorldID getId() const { return id_; }
    const glm::vec3& getMineCenter() const { return mineCenter_; }
    const glm::vec3& getSurfaceCenter() const { return surfaceCenter_; }
    float getMineRadius() const { return mineRadius_; }
    float getSurfaceRadius() const { return surfaceRadius_; }
    uint64_t getWorldTickCount() const { return tickCount_; }

private:
    void spawnResources();
    void updatePhysics(float deltaTime);
    void checkCollisions();
    void updateStructures(float deltaTime);
    void respawnResources(float deltaTime);
    void cleanupDeadPlayers();

    WorldID id_;
    std::vector<Player*> players_;
    std::map<PlayerID, std::unique_ptr<Player>> playerMap_;
    std::vector<std::unique_ptr<Structure>> structures_;
    std::vector<std::unique_ptr<Resource>> resources_;
    std::unique_ptr<Physics> physics_;
    std::unique_ptr<MiningSystem> miningSystem_;
    std::unique_ptr<BuildingSystem> buildingSystem_;
    std::unique_ptr<CombatSystem> combatSystem_;

    // World parameters
    glm::vec3 mineCenter_ = glm::vec3(0, -50, 0);
    glm::vec3 surfaceCenter_ = glm::vec3(0, 0, 0);
    float mineRadius_ = 100.0f;
    float surfaceRadius_ = 500.0f;
    
    StructureID nextStructureId_ = 1;
    ResourceID nextResourceId_ = 1;
    uint64_t tickCount_ = 0;
    
    float resourceSpawnTimer_ = 0.0f;
    float resourceSpawnInterval_ = 5.0f;  // Spawn resources every 5 seconds
    uint32_t maxResourcesPerSpawn_ = 8;
    
    // Performance
    static constexpr size_t MAX_PLAYERS_PER_WORLD = 100;
    static constexpr float WORLD_UPDATE_INTERVAL = 1.0f / 60.0f;  // 60 Hz
};
