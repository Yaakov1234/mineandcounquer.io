#include "MessageHandler.h"
#include <iostream>
#include <glm/glm.hpp>

MessageHandler::MessageHandler(World* world)
    : world_(world) {
}

bool MessageHandler::handlePlayerMove(PlayerID playerId, const json& payload) {
    if (!world_) return false;
    
    Player* player = world_->getPlayer(playerId);
    if (!player || !player->isAlive()) return false;

    try {
        glm::vec3 velocity(
            payload["velocity"]["x"].get<float>(),
            payload["velocity"]["y"].get<float>(),
            payload["velocity"]["z"].get<float>()
        );
        
        player->setVelocity(velocity);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error handling player move: " << e.what() << std::endl;
        return false;
    }
}

bool MessageHandler::handlePlayerMine(PlayerID playerId, const json& payload) {
    if (!world_) return false;
    
    Player* player = world_->getPlayer(playerId);
    if (!player || !player->isAlive() || !player->isInMine()) return false;

    // Collect nearby resources
    auto nearby = world_->getNearbyResources(player->getPosition(), 15.0f);
    bool collected = false;
    
    for (ResourceID resId : nearby) {
        Resource* resource = world_->getResource(resId);
        if (resource && !resource->collected) {
            player->addResource(resource->type, resource->value);
            resource->collected = true;
            collected = true;
            std::cout << "Player " << playerId << " collected resource " << resId << std::endl;
        }
    }
    
    return collected;
}

bool MessageHandler::handlePlayerFireWeapon(PlayerID playerId, const json& payload) {
    if (!world_) return false;
    
    Player* player = world_->getPlayer(playerId);
    if (!player || !player->isAlive()) return false;

    try {
        WeaponType weapon = static_cast<WeaponType>(payload["weapon"].get<uint32_t>());
        glm::vec3 targetPos(
            payload["target"]["x"].get<float>(),
            payload["target"]["y"].get<float>(),
            payload["target"]["z"].get<float>()
        );
        
        bool hit = world_->fireWeapon(playerId, targetPos, weapon);
        if (hit) {
            // Remove weapon ammo
            player->removeWeapon(weapon, 1);
        }
        
        return hit;
    } catch (const std::exception& e) {
        std::cerr << "Error handling fire weapon: " << e.what() << std::endl;
        return false;
    }
}

bool MessageHandler::handlePlayerPlaceStructure(PlayerID playerId, const json& payload) {
    if (!world_) return false;
    
    Player* player = world_->getPlayer(playerId);
    if (!player || !player->isAlive() || player->isInMine()) return false;

    try {
        glm::vec3 position(
            payload["position"]["x"].get<float>(),
            payload["position"]["y"].get<float>(),
            payload["position"]["z"].get<float>()
        );
        StructureType type = static_cast<StructureType>(payload["type"].get<uint32_t>());
        
        // Check resources
        uint32_t gemCost = 10;  // TODO: Get from building system
        uint32_t rockCost = 20;
        
        if (!player->removeResource(ResourceType::GEM, gemCost) ||
            !player->removeResource(ResourceType::ROCK, rockCost)) {
            std::cout << "Player " << playerId << " insufficient resources to place structure" << std::endl;
            return false;
        }
        
        StructureID structId = world_->addStructure(playerId, position, type);
        std::cout << "Player " << playerId << " placed structure " << structId << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error handling place structure: " << e.what() << std::endl;
        return false;
    }
}

bool MessageHandler::handlePlayerExitMine(PlayerID playerId, const json& payload) {
    if (!world_) return false;
    
    Player* player = world_->getPlayer(playerId);
    if (!player || !player->isAlive() || !player->isInMine()) return false;

    // Check if player has resources
    if (!world_->canExitMine(playerId)) {
        std::cout << "Player " << playerId << " cannot exit mine (no resources)" << std::endl;
        return false;
    }
    
    world_->movePlayerToSurface(playerId);
    return true;
}

bool MessageHandler::handlePlayerEnterMine(PlayerID playerId, const json& payload) {
    if (!world_) return false;
    
    Player* player = world_->getPlayer(playerId);
    if (!player || !player->isAlive()) return false;

    world_->movePlayerToMine(playerId);
    return true;
}

bool MessageHandler::handlePlayerBuyWeapon(PlayerID playerId, const json& payload) {
    if (!world_) return false;
    
    Player* player = world_->getPlayer(playerId);
    if (!player || !player->isAlive()) return false;

    try {
        WeaponType weapon = static_cast<WeaponType>(payload["weapon"].get<uint32_t>());
        uint32_t cost = 50;  // TODO: Get from economy system
        
        if (player->getInventory().gems < cost) {
            std::cout << "Player " << playerId << " insufficient gems to buy weapon" << std::endl;
            return false;
        }
        
        player->removeResource(ResourceType::GEM, cost);
        player->addWeapon(weapon, 1);
        
        std::cout << "Player " << playerId << " purchased weapon " << static_cast<int>(weapon) << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error handling buy weapon: " << e.what() << std::endl;
        return false;
    }
}

json MessageHandler::serializeWorldState(const World& world) {
    json state;
    state["tickCount"] = world.getWorldTickCount();
    
    // Serialize all players
    state["players"] = json::array();
    for (const auto* player : world.getAllPlayers()) {
        if (player) {
            state["players"].push_back(serializePlayerState(*player));
        }
    }
    
    // Serialize all structures
    state["structures"] = json::array();
    for (const auto& structure : world.getAllStructures()) {
        if (structure) {
            state["structures"].push_back(serializeStructureState(*structure));
        }
    }
    
    // Serialize all resources
    state["resources"] = json::array();
    for (const auto& resource : world.getAllResources()) {
        if (resource) {
            state["resources"].push_back(serializeResourceState(*resource));
        }
    }
    
    return state;
}

json MessageHandler::serializePlayerState(const Player& player) {
    const auto& state = player.getState();
    json j;
    j["id"] = player.getId();
    j["username"] = player.getUsername();
    j["position"] = {
        {"x", player.getPosition().x},
        {"y", player.getPosition().y},
        {"z", player.getPosition().z}
    };
    j["velocity"] = {
        {"x", player.getVelocity().x},
        {"y", player.getVelocity().y},
        {"z", player.getVelocity().z}
    };
    j["health"] = player.getHealth();
    j["maxHealth"] = player.getMaxHealth();
    j["kills"] = player.getKills();
    j["deaths"] = player.getDeaths();
    j["territory"] = player.getTerritoryControlled();
    j["inMine"] = player.isInMine();
    j["alive"] = player.isAlive();
    j["skinId"] = player.getSkinId();
    j["streak"] = player.getStreak();
    j["inventory"] = serializeInventory(player.getInventory());
    
    return j;
}

json MessageHandler::serializeStructureState(const Structure& structure) {
    json j;
    j["id"] = structure.id;
    j["ownerId"] = structure.ownerId;
    j["position"] = {
        {"x", structure.position.x},
        {"y", structure.position.y},
        {"z", structure.position.z}
    };
    j["type"] = static_cast<uint32_t>(structure.type);
    j["health"] = structure.health;
    j["maxHealth"] = structure.maxHealth;
    j["destroyed"] = structure.isDestroyed;
    j["buildProgress"] = structure.buildProgress;
    
    return j;
}

json MessageHandler::serializeResourceState(const Resource& resource) {
    json j;
    j["id"] = resource.id;
    j["type"] = static_cast<uint32_t>(resource.type);
    j["position"] = {
        {"x", resource.position.x},
        {"y", resource.position.y},
        {"z", resource.position.z}
    };
    j["value"] = resource.value;
    j["collected"] = resource.collected;
    
    return j;
}

json MessageHandler::serializeInventory(const Inventory& inventory) {
    json j;
    j["gems"] = inventory.gems;
    j["rocks"] = inventory.rocks;
    j["weapons"] = json::object();
    
    for (const auto& [weapon, count] : inventory.weapons) {
        std::string weaponName;
        switch (weapon) {
            case WeaponType::PISTOL: weaponName = "pistol"; break;
            case WeaponType::RIFLE: weaponName = "rifle"; break;
            case WeaponType::SHOTGUN: weaponName = "shotgun"; break;
            case WeaponType::SNIPER: weaponName = "sniper"; break;
            case WeaponType::ROCKET_LAUNCHER: weaponName = "rocket"; break;
        }
        j["weapons"][weaponName] = count;
    }
    
    return j;
}

bool MessageHandler::validatePosition(const glm::vec3& pos, const World& world) const {
    // TODO: Validate position is within world bounds
    return true;
}

bool MessageHandler::validateInventory(PlayerID playerId) const {
    // TODO: Validate inventory consistency
    return true;
}
