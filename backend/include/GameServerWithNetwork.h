#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include "GameServer.h"
#include "WebSocketServer.h"
#include "MessageHandler.h"

class GameServerWithNetwork : public GameServer {
public:
    GameServerWithNetwork(uint16_t gamePort = 8080, uint16_t wsPort = 9090);
    ~GameServerWithNetwork() = default;

    // Server lifecycle
    void start() override;
    void stop() override;
    void update(float deltaTime) override;

    // Network integration
    WebSocketServer* getWebSocketServer() { return wsServer_.get(); }
    MessageHandler* getMessageHandler() { return msgHandler_.get(); }

private:
    std::unique_ptr<WebSocketServer> wsServer_;
    std::unique_ptr<MessageHandler> msgHandler_;
    
    void processNetworkMessages();
    void broadcastWorldState();
    
    float stateUpdateTimer_ = 0.0f;
    float stateUpdateInterval_ = 0.1f;  // Send state every 100ms
};
