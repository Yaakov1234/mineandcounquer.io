#include "World.h"
#include "MiningSystem.h"
#include <random>
#include <glm/gtc/random.hpp>
#include <iostream>
#include <algorithm>

World::World(WorldID id)
    : id_(id), 
      physics_(std::make_unique<Physics>()),
      miningSystem_(std::make_unique<MiningSystem>()),
      buildingSystem_(std::make_unique<BuildingSystem>()),
      combatSystem_(std::make_unique<CombatSystem>()) {
    std::cout << "World " << id << " initialized" << std::endl;
    spawnResources();  // Initial resource spawn
}

World::~World() = default;

void World::update(float deltaTime) {
    tickCount_++;
    
    // Update all players
    for (auto* player : players_) {
        if (player) {
            player->update(deltaTime);
        }
    }

    // Update physics
    updatePhysics(deltaTime);

    // Check collisions and resource collection
    checkCollisions();

    // Update structures (building progress, decay, etc.)
    updateStructures(deltaTime);

    // Respawn resources periodically
    respawnResources(deltaTime);
    
    // Clean up dead players
    cleanupDeadPlayers();
}

PlayerID World::addPlayer(const std::string& username) {
    static PlayerID nextId = 1;
    PlayerID id = nextId++;

    glm::vec3 spawnPos = getRandomMineSpawn();
    auto player = std::make_unique<Player>(id, username, spawnPos);
    Player* playerPtr = player.get();
    
    playerMap_[id] = std::move(player);
    players_.push_back(playerPtr);

    std::cout << "Player " << username << " (ID: " << id << ") spawned in mine at (" 
              << spawnPos.x << ", " << spawnPos.y << ", " << spawnPos.z << ")" << std::endl;

    return id;
}

void World::removePlayer(PlayerID id) {
    auto it = playerMap_.find(id);
    if (it != playerMap_.end()) {
        // Remove from players vector
        players_.erase(std::remove(players_.begin(), players_.end(), it->second.get()), players_.end());
        playerMap_.erase(it);
        std::cout << "Player ID " << id << " removed from world" << std::endl;
    }
}

Player* World::getPlayer(PlayerID id) {
    auto it = playerMap_.find(id);
    if (it != playerMap_.end()) {
        return it->second.get();
    }
    return nullptr;
}

StructureID World::addStructure(PlayerID ownerId, const glm::vec3& pos, StructureType type) {
    if (!buildingSystem_->canPlaceStructure(pos, type)) {
        std::cout << "Cannot place structure at position" << std::endl;
        return 0;
    }

    StructureID id = nextStructureId_++;
    auto structure = std::make_unique<Structure>();
    structure->id = id;
    structure->ownerId = ownerId;
    structure->position = pos;
    structure->type = type;
    structure->health = buildingSystem_->getStructureHealth(type);
    structure->maxHealth = structure->health;
    structure->buildTimeRemaining = buildingSystem_->getBuildTimeMs(type);
    structure->buildProgress = 0.0f;

    structures_.push_back(std::move(structure));
    std::cout << "Structure " << id << " placed at (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
    
    return id;
}

void World::removeStructure(StructureID id) {
    structures_.erase(
        std::remove_if(structures_.begin(), structures_.end(),
            [id](const std::unique_ptr<Structure>& s) { return s->id == id; }),
        structures_.end()
    );
}

Structure* World::getStructure(StructureID id) {
    for (auto& structure : structures_) {
        if (structure->id == id) {
            return structure.get();
        }
    }
    return nullptr;
}

std::vector<StructureID> World::getPlayerStructures(PlayerID ownerId) const {
    std::vector<StructureID> playerStructures;
    for (const auto& structure : structures_) {
        if (structure->ownerId == ownerId && !structure->isDestroyed) {
            playerStructures.push_back(structure->id);
        }
    }
    return playerStructures;
}

uint64_t World::getPlayerTerritory(PlayerID ownerId) const {
    uint64_t territory = 0;
    for (const auto& structure : structures_) {
        if (structure->ownerId == ownerId && !structure->isDestroyed && structure->buildProgress >= 1.0f) {
            territory += buildingSystem_->calculateTerritorySize(structure->type);
        }
    }
    return territory;
}

ResourceID World::addResource(ResourceType type, const glm::vec3& pos, uint32_t value) {
    ResourceID id = nextResourceId_++;
    auto resource = std::make_unique<Resource>();
    resource->id = id;
    resource->type = type;
    resource->position = pos;
    resource->value = value;
    resource->collected = false;

    resources_.push_back(std::move(resource));
    return id;
}

void World::removeResource(ResourceID id) {
    resources_.erase(
        std::remove_if(resources_.begin(), resources_.end(),
            [id](const std::unique_ptr<Resource>& r) { return r->id == id; }),
        resources_.end()
    );
}

Resource* World::getResource(ResourceID id) {
    for (auto& resource : resources_) {
        if (resource->id == id) {
            return resource.get();
        }
    }
    return nullptr;
}

std::vector<ResourceID> World::getNearbyResources(const glm::vec3& pos, float radius) const {
    std::vector<ResourceID> nearby;
    for (const auto& resource : resources_) {
        if (!resource->collected && glm::distance(pos, resource->position) <= radius) {
            nearby.push_back(resource->id);
        }
    }
    return nearby;
}

bool World::canExitMine(PlayerID playerId) const {
    Player* player = const_cast<Player*>(const_cast<const World*>(this)->getPlayer(playerId));
    if (!player) return false;
    
    // Need at least some resources to exit mine
    return player->getInventory().gems > 0 || player->getInventory().rocks > 0;
}

void World::movePlayerToSurface(PlayerID playerId) {
    Player* player = getPlayer(playerId);
    if (player) {
        player->enterZone(ZoneType::SURFACE);
        glm::vec3 spawnPos = getRandomSurfaceSpawn();
        player->setPosition(spawnPos);
        std::cout << "Player " << playerId << " moved to surface" << std::endl;
    }
}

void World::movePlayerToMine(PlayerID playerId) {
    Player* player = getPlayer(playerId);
    if (player) {
        player->enterZone(ZoneType::MINE);
        glm::vec3 spawnPos = getRandomMineSpawn();
        player->setPosition(spawnPos);
        std::cout << "Player " << playerId << " moved to mine" << std::endl;
    }
}

bool World::fireWeapon(PlayerID shooterId, const glm::vec3& targetPos, WeaponType weapon) {
    Player* shooter = getPlayer(shooterId);
    if (!shooter || !shooter->isAlive()) return false;

    // Check weapon availability
    if (shooter->getInventory().getWeaponCount(weapon) == 0) {
        return false;
    }

    // Check range
    if (!combatSystem_->canHit(shooter->getPosition(), targetPos, weapon)) {
        return false;
    }

    // Calculate damage
    float distance = glm::distance(shooter->getPosition(), targetPos);
    float damage = combatSystem_->calculateDamage(weapon, distance);

    // Check for player hits
    for (auto* target : players_) {
        if (target && target->isAlive() && target->getId() != shooterId) {
            if (glm::distance(target->getPosition(), targetPos) < 5.0f) {
                // Apply armor reduction
                if (target->hasArmor()) {
                    damage = combatSystem_->calculateArmorReduction(damage, true);
                }
                damagePlayer(target->getId(), damage);
                return true;
            }
        }
    }

    // Check for structure hits
    for (auto& structure : structures_) {
        if (glm::distance(structure->position, targetPos) < 10.0f) {
            float structureDamage = combatSystem_->getStructureDamage(damage);
            damageStructure(structure->id, structureDamage);
            return true;
        }
    }

    return false;
}

void World::damagePlayer(PlayerID targetId, float damage) {
    Player* target = getPlayer(targetId);
    if (target && target->isAlive()) {
        target->applyDamage(damage);
        std::cout << "Player " << targetId << " took " << damage << " damage" << std::endl;
    }
}

void World::damageStructure(StructureID structureId, float damage) {
    Structure* structure = getStructure(structureId);
    if (structure && !structure->isDestroyed) {
        combatSystem_->damageStructure(*structure, damage);
        std::cout << "Structure " << structureId << " took " << damage << " damage" << std::endl;
    }
}

void World::killPlayer(PlayerID playerId, PlayerID killerId) {
    Player* victim = getPlayer(playerId);
    Player* killer = getPlayer(killerId);

    if (victim) {
        victim->recordDeath();
        std::cout << "Player " << playerId << " died" << std::endl;
    }

    if (killer) {
        killer->recordKill();
        std::cout << "Player " << killerId << " got a kill (Streak: " << killer->getStreak() << ")" << std::endl;
    }
}

glm::vec3 World::getRandomMineSpawn() const {
    glm::vec3 randomOffset = glm::ballRand(mineRadius_);
    return mineCenter_ + randomOffset;
}

glm::vec3 World::getRandomSurfaceSpawn() const {
    glm::vec3 randomOffset = glm::ballRand(surfaceRadius_);
    return surfaceCenter_ + randomOffset;
}

void World::spawnResources() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> gemDis(1, 3);
    std::uniform_int_distribution<> rockDis(1, 5);

    // Spawn gems in clusters
    for (uint32_t i = 0; i < 3; ++i) {
        glm::vec3 pos = getRandomMineSpawn();
        uint32_t rarity = gemDis(gen);
        addResource(ResourceType::GEM, pos, rarity);
    }

    // Spawn rocks in clusters
    for (uint32_t i = 0; i < 5; ++i) {
        glm::vec3 pos = getRandomMineSpawn();
        uint32_t amount = rockDis(gen);
        addResource(ResourceType::ROCK, pos, amount);
    }
}

void World::updatePhysics(float deltaTime) {
    // Update player positions based on velocity
    for (auto* player : players_) {
        if (player && player->isAlive()) {
            glm::vec3 newPos = player->getPosition() + player->getVelocity() * deltaTime;
            
            // Clamp to world bounds
            float maxRadius = player->isInMine() ? mineRadius_ : surfaceRadius_;
            glm::vec3 center = player->isInMine() ? mineCenter_ : surfaceCenter_;
            
            if (glm::distance(newPos, center) <= maxRadius) {
                player->setPosition(newPos);
            }
        }
    }
}

void World::checkCollisions() {
    // Resource collection
    for (auto* player : players_) {
        if (!player || !player->isAlive()) continue;
        
        auto nearby = getNearbyResources(player->getPosition(), 10.0f);
        for (ResourceID resId : nearby) {
            Resource* resource = getResource(resId);
            if (resource && !resource->collected) {
                player->addResource(resource->type, resource->value);
                resource->collected = true;
                std::cout << "Player " << player->getId() << " collected resource " << resId << std::endl;
            }
        }
    }
}

void World::updateStructures(float deltaTime) {
    for (auto& structure : structures_) {
        if (!structure || structure->isDestroyed) continue;
        
        // Update building progress
        if (structure->buildProgress < 1.0f) {
            structure->buildTimeRemaining -= static_cast<uint64_t>(deltaTime * 1000);
            if (structure->buildTimeRemaining <= 0) {
                structure->buildProgress = 1.0f;
                std::cout << "Structure " << structure->id << " completed" << std::endl;
            } else {
                structure->buildProgress = 1.0f - (static_cast<float>(structure->buildTimeRemaining) / 
                    buildingSystem_->getBuildTimeMs(structure->type));
            }
        }
    }
}

void World::respawnResources(float deltaTime) {
    resourceSpawnTimer_ += deltaTime;
    if (resourceSpawnTimer_ >= resourceSpawnInterval_) {
        // Remove old collected resources and respawn
        resources_.erase(
            std::remove_if(resources_.begin(), resources_.end(),
                [](const std::unique_ptr<Resource>& r) { return r->collected; }),
            resources_.end()
        );
        
        spawnResources();
        resourceSpawnTimer_ = 0.0f;
    }
}

void World::cleanupDeadPlayers() {
    for (auto* player : players_) {
        if (player && !player->isAlive()) {
            // Respawn player in mine
            player->setPosition(getRandomMineSpawn());
            // Reset health
            player->heal(player->getMaxHealth());
        }
    }
}
