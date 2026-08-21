#pragma once

#include <nlohmann/json.hpp>
#include "common/Types.h"
#include "World.h"

using json = nlohmann::json;

class MessageHandler {
public:
    MessageHandler(World* world);
    ~MessageHandler() = default;

    // Handle incoming messages from clients
    bool handlePlayerMove(PlayerID playerId, const json& payload);
    bool handlePlayerMine(PlayerID playerId, const json& payload);
    bool handlePlayerFireWeapon(PlayerID playerId, const json& payload);
    bool handlePlayerPlaceStructure(PlayerID playerId, const json& payload);
    bool handlePlayerExitMine(PlayerID playerId, const json& payload);
    bool handlePlayerEnterMine(PlayerID playerId, const json& payload);
    bool handlePlayerBuyWeapon(PlayerID playerId, const json& payload);

    // World state serialization for clients
    json serializeWorldState(const World& world);
    json serializePlayerState(const Player& player);
    json serializeStructureState(const Structure& structure);
    json serializeResourceState(const Resource& resource);
    json serializeInventory(const Inventory& inventory);

private:
    World* world_;
    
    // Validation helpers
    bool validatePosition(const glm::vec3& pos, const World& world) const;
    bool validateInventory(PlayerID playerId) const;
};
