#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <queue>
#include <nlohmann/json.hpp>
#include "common/Types.h"

using json = nlohmann::json;

enum class MessageType {
    // Client -> Server
    PLAYER_JOIN,
    PLAYER_MOVE,
    PLAYER_MINE,
    PLAYER_FIRE_WEAPON,
    PLAYER_PLACE_STRUCTURE,
    PLAYER_EXIT_MINE,
    PLAYER_ENTER_MINE,
    PLAYER_BUY_WEAPON,
    PLAYER_DISCONNECT,
    PING,
    
    // Server -> Client
    PLAYER_JOINED,
    WORLD_STATE,
    PLAYER_UPDATE,
    STRUCTURE_UPDATE,
    RESOURCE_UPDATE,
    PLAYER_DIED,
    PLAYER_KILLED,
    STRUCTURE_DESTROYED,
    ACHIEVEMENT_UNLOCKED,
    PONG,
    ERROR_MSG
};

struct Message {
    MessageType type;
    json payload;
    uint32_t playerId = 0;
    uint64_t timestamp = 0;
};

class NetworkManager {
public:
    NetworkManager(uint16_t port = 8080);
    ~NetworkManager();

    // Server management
    bool initialize();
    void shutdown();
    void update(float deltaTime);
    bool isRunning() const { return running_; }

    // Message handling
    void sendToPlayer(PlayerID playerId, const Message& msg);
    void broadcastToWorld(WorldID worldId, const Message& msg);
    std::vector<Message> getIncomingMessages();
    void sendWorldState(WorldID worldId, const json& state);
    
    // Connection management
    void onPlayerConnected(PlayerID playerId);
    void onPlayerDisconnected(PlayerID playerId);
    bool isPlayerConnected(PlayerID playerId) const;
    
    // JSON serialization helpers
    static json serializePlayer(const PlayerState& player);
    static json serializeStructure(const Structure& structure);
    static json serializeResource(const Resource& resource);
    static json serializeInventory(const Inventory& inv);

private:
    uint16_t port_;
    bool running_ = false;
    std::queue<Message> incomingMessages_;
    std::vector<PlayerID> connectedPlayers_;
    float messageBufferTime_ = 0.0f;
    float messageFlushInterval_ = 0.016f;  // ~60 Hz
    
    // WebSocket implementation will go here
    // For now, this is a placeholder for the message queue system
};
