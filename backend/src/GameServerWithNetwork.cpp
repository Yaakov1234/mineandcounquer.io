#include "GameServerWithNetwork.h"
#include <iostream>
#include <algorithm>

GameServerWithNetwork::GameServerWithNetwork(uint16_t gamePort, uint16_t wsPort)
    : GameServer(gamePort) {
    wsServer_ = std::make_unique<WebSocketServer>(wsPort);
    msgHandler_ = std::make_unique<MessageHandler>(nullptr);  // Will set world later
}

void GameServerWithNetwork::start() {
    std::cout << "Starting Game Server with Network Integration..." << std::endl;
    
    // Start base game server
    GameServer::start();
    
    // Start WebSocket server
    if (!wsServer_->start()) {
        std::cerr << "Failed to start WebSocket server" << std::endl;
        stop();
        return;
    }
    
    std::cout << "Game Server with Network started successfully" << std::endl;
}

void GameServerWithNetwork::stop() {
    std::cout << "Stopping Game Server with Network..." << std::endl;
    
    if (wsServer_) {
        wsServer_->stop();
    }
    
    GameServer::stop();
}

void GameServerWithNetwork::update(float deltaTime) {
    // Update base game server
    GameServer::update(deltaTime);
    
    // Update network
    if (wsServer_) {
        wsServer_->update();
        processNetworkMessages();
    }
    
    // Broadcast world state periodically
    stateUpdateTimer_ += deltaTime;
    if (stateUpdateTimer_ >= stateUpdateInterval_) {
        broadcastWorldState();
        stateUpdateTimer_ = 0.0f;
    }
}

void GameServerWithNetwork::processNetworkMessages() {
    if (!wsServer_) return;
    
    auto messages = wsServer_->getIncomingMessages();
    
    for (const auto& msg : messages) {
        std::cout << "Processing message type: " << static_cast<int>(msg.type) << std::endl;
        
        // Get the world for this player
        ClientConnection* conn = wsServer_->getConnection(msg.connId);
        if (!conn || conn->worldId == 0) {
            std::cout << "Invalid connection or world ID" << std::endl;
            continue;
        }
        
        World* world = getWorld(conn->worldId);
        if (!world) {
            std::cout << "World not found: " << conn->worldId << std::endl;
            continue;
        }
        
        // Update message handler's world reference
        msgHandler_ = std::make_unique<MessageHandler>(world);
        
        // Handle different message types
        switch (msg.type) {
            case MessageType::PLAYER_JOIN: {
                // Add player to world
                std::string username = msg.payload["username"].get<std::string>();
                PlayerID playerId = world->addPlayer(username);
                wsServer_->setPlayerForConnection(msg.connId, playerId, conn->worldId);
                
                // Send player joined message
                Message response;
                response.type = MessageType::PLAYER_JOINED;
                response.payload["playerId"] = playerId;
                response.payload["position"] = {
                    {"x", world->getPlayer(playerId)->getPosition().x},
                    {"y", world->getPlayer(playerId)->getPosition().y},
                    {"z", world->getPlayer(playerId)->getPosition().z}
                };
                wsServer_->sendToConnection(msg.connId, response);
                
                // Broadcast to other players
                Message broadcast = response;
                broadcast.type = MessageType::WORLD_STATE;
                broadcast.payload = msgHandler_->serializeWorldState(*world);
                wsServer_->broadcastToWorld(conn->worldId, broadcast);
                break;
            }
            case MessageType::PLAYER_MOVE:
                msgHandler_->handlePlayerMove(msg.playerId, msg.payload);
                break;
            case MessageType::PLAYER_MINE:
                msgHandler_->handlePlayerMine(msg.playerId, msg.payload);
                break;
            case MessageType::PLAYER_FIRE_WEAPON:
                msgHandler_->handlePlayerFireWeapon(msg.playerId, msg.payload);
                break;
            case MessageType::PLAYER_PLACE_STRUCTURE:
                msgHandler_->handlePlayerPlaceStructure(msg.playerId, msg.payload);
                break;
            case MessageType::PLAYER_EXIT_MINE:
                msgHandler_->handlePlayerExitMine(msg.playerId, msg.payload);
                break;
            case MessageType::PLAYER_ENTER_MINE:
                msgHandler_->handlePlayerEnterMine(msg.playerId, msg.payload);
                break;
            case MessageType::PLAYER_BUY_WEAPON:
                msgHandler_->handlePlayerBuyWeapon(msg.playerId, msg.payload);
                break;
            case MessageType::PLAYER_DISCONNECT:
                wsServer_->unregisterConnection(msg.connId);
                world->removePlayer(msg.playerId);
                break;
            case MessageType::PING:
                {
                    Message pong;
                    pong.type = MessageType::PONG;
                    wsServer_->sendToConnection(msg.connId, pong);
                }
                break;
            default:
                std::cout << "Unknown message type" << std::endl;
                break;
        }
    }
}

void GameServerWithNetwork::broadcastWorldState() {
    if (!wsServer_ || !msgHandler_) return;
    
    // Get all worlds and broadcast their state to connected clients
    // This is a simplified version - iterate through all worlds
    // In a real implementation, you'd track which worlds have active players
    
    // For now, just broadcast a sync tick to keep clients in sync
    Message syncMsg;
    syncMsg.type = MessageType::SYNC_TICK;
    syncMsg.payload["tick"] = wsServer_->tickCounter_;
    
    wsServer_->broadcastToAll(syncMsg);
}
