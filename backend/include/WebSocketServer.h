#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <queue>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include "common/Types.h"

using json = nlohmann::json;

// Connection ID for tracking clients
using ConnectionID = uint64_t;

enum class MessageType {
    // Client -> Server
    PLAYER_JOIN,           // New player joining
    PLAYER_MOVE,           // Player movement input
    PLAYER_MINE,           // Mining action
    PLAYER_FIRE_WEAPON,    // Fire weapon at target
    PLAYER_PLACE_STRUCTURE,// Place structure
    PLAYER_EXIT_MINE,      // Exit to surface
    PLAYER_ENTER_MINE,     // Return to mine
    PLAYER_BUY_WEAPON,     // Buy weapon from dealer
    PLAYER_DISCONNECT,     // Intentional disconnect
    PING,                  // Keep-alive ping
    
    // Server -> Client
    PLAYER_JOINED,         // Player spawned
    WORLD_STATE,           // Full world state
    PLAYER_UPDATE,         // Player state delta
    STRUCTURE_UPDATE,      // Structure state update
    RESOURCE_UPDATE,       // Resource spawned/collected
    PLAYER_DIED,           // Player died
    PLAYER_KILLED,         // Player killed another
    STRUCTURE_DESTROYED,   // Structure destroyed
    ACHIEVEMENT_UNLOCKED,  // Achievement unlocked
    INVENTORY_UPDATE,      // Inventory changed
    PONG,                  // Keep-alive pong
    ERROR_MSG,             // Error message
    SYNC_TICK              // Server tick for synchronization
};

struct Message {
    MessageType type;
    json payload;
    PlayerID playerId = 0;
    ConnectionID connId = 0;
    uint64_t timestamp = 0;
};

struct ClientConnection {
    ConnectionID id;
    PlayerID playerId = 0;
    WorldID worldId = 0;
    bool authenticated = false;
    uint64_t lastPingTime = 0;
    uint64_t connectionTime = 0;
    std::string username;
};

class WebSocketServer {
public:
    WebSocketServer(uint16_t port = 8080);
    ~WebSocketServer();

    // Server lifecycle
    bool start();
    void stop();
    void update();
    bool isRunning() const { return running_; }

    // Message sending
    void sendToConnection(ConnectionID connId, const Message& msg);
    void broadcastToWorld(WorldID worldId, const Message& msg);
    void broadcastToAll(const Message& msg);

    // Connection management
    ClientConnection* getConnection(ConnectionID connId);
    std::vector<ConnectionID> getWorldConnections(WorldID worldId) const;
    size_t getConnectionCount() const { return connections_.size(); }

    // Message handling
    std::vector<Message> getIncomingMessages();
    void setMessageCallback(std::function<void(const Message&)> callback);

    // Connection tracking
    void registerConnection(ConnectionID connId, const std::string& username);
    void unregisterConnection(ConnectionID connId);
    void setPlayerForConnection(ConnectionID connId, PlayerID playerId, WorldID worldId);

private:
    uint16_t port_;
    bool running_ = false;
    
    std::unordered_map<ConnectionID, std::unique_ptr<ClientConnection>> connections_;
    std::queue<Message> incomingMessages_;
    std::function<void(const Message&)> messageCallback_;
    
    // Message buffering for batching
    std::unordered_map<ConnectionID, std::vector<Message>> outgoingBuffers_;
    float messageFlushInterval_ = 0.016f;  // ~60 Hz
    float messageBufferTime_ = 0.0f;
    uint64_t tickCounter_ = 0;
    
    // Internal state
    std::unordered_map<WorldID, std::vector<ConnectionID>> worldConnections_;
    
    void flushMessages();
    void handleConnection(ConnectionID connId);
    void handleDisconnection(ConnectionID connId);
    
    // Helper methods
    ConnectionID generateConnectionId();
    uint64_t getCurrentTimestamp() const;
};
