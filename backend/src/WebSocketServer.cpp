#include "WebSocketServer.h"
#include <iostream>
#include <chrono>
#include <algorithm>

WebSocketServer::WebSocketServer(uint16_t port)
    : port_(port) {
}

WebSocketServer::~WebSocketServer() {
    stop();
}

bool WebSocketServer::start() {
    std::cout << "Starting WebSocket server on port " << port_ << std::endl;
    running_ = true;
    
    // TODO: Initialize actual WebSocket server (websocketpp or similar)
    // For now, this is a placeholder that simulates the server
    
    std::cout << "WebSocket server started successfully" << std::endl;
    return true;
}

void WebSocketServer::stop() {
    std::cout << "Stopping WebSocket server..." << std::endl;
    running_ = false;
    connections_.clear();
    worldConnections_.clear();
}

void WebSocketServer::update() {
    messageBufferTime_ += 0.016f;  // Assume 60 Hz update
    tickCounter_++;
    
    // Flush messages periodically
    if (messageBufferTime_ >= messageFlushInterval_) {
        flushMessages();
        messageBufferTime_ = 0.0f;
    }
}

void WebSocketServer::sendToConnection(ConnectionID connId, const Message& msg) {
    if (!running_) return;
    
    auto it = connections_.find(connId);
    if (it != connections_.end()) {
        outgoingBuffers_[connId].push_back(msg);
    }
}

void WebSocketServer::broadcastToWorld(WorldID worldId, const Message& msg) {
    if (!running_) return;
    
    auto it = worldConnections_.find(worldId);
    if (it != worldConnections_.end()) {
        for (ConnectionID connId : it->second) {
            sendToConnection(connId, msg);
        }
    }
}

void WebSocketServer::broadcastToAll(const Message& msg) {
    if (!running_) return;
    
    for (const auto& [connId, _] : connections_) {
        sendToConnection(connId, msg);
    }
}

ClientConnection* WebSocketServer::getConnection(ConnectionID connId) {
    auto it = connections_.find(connId);
    if (it != connections_.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<ConnectionID> WebSocketServer::getWorldConnections(WorldID worldId) const {
    auto it = worldConnections_.find(worldId);
    if (it != worldConnections_.end()) {
        return it->second;
    }
    return {};
}

std::vector<Message> WebSocketServer::getIncomingMessages() {
    std::vector<Message> messages;
    while (!incomingMessages_.empty()) {
        messages.push_back(incomingMessages_.front());
        incomingMessages_.pop();
    }
    return messages;
}

void WebSocketServer::setMessageCallback(std::function<void(const Message&)> callback) {
    messageCallback_ = callback;
}

void WebSocketServer::registerConnection(ConnectionID connId, const std::string& username) {
    auto connection = std::make_unique<ClientConnection>();
    connection->id = connId;
    connection->username = username;
    connection->connectionTime = getCurrentTimestamp();
    
    connections_[connId] = std::move(connection);
    std::cout << "Connection " << connId << " registered (" << username << ")" << std::endl;
}

void WebSocketServer::unregisterConnection(ConnectionID connId) {
    auto it = connections_.find(connId);
    if (it != connections_.end()) {
        ClientConnection* conn = it->second.get();
        
        // Remove from world connections
        if (conn->worldId > 0) {
            auto worldIt = worldConnections_.find(conn->worldId);
            if (worldIt != worldConnections_.end()) {
                auto& conns = worldIt->second;
                conns.erase(std::remove(conns.begin(), conns.end(), connId), conns.end());
            }
        }
        
        // Remove from connections
        outgoingBuffers_.erase(connId);
        connections_.erase(it);
        
        std::cout << "Connection " << connId << " unregistered" << std::endl;
    }
}

void WebSocketServer::setPlayerForConnection(ConnectionID connId, PlayerID playerId, WorldID worldId) {
    auto it = connections_.find(connId);
    if (it != connections_.end()) {
        ClientConnection* conn = it->second.get();
        conn->playerId = playerId;
        conn->worldId = worldId;
        conn->authenticated = true;
        
        // Add to world connections
        worldConnections_[worldId].push_back(connId);
        
        std::cout << "Player " << playerId << " assigned to connection " << connId 
                  << " in world " << worldId << std::endl;
    }
}

void WebSocketServer::flushMessages() {
    // TODO: Send all buffered messages to clients via WebSocket
    // This would involve serializing JSON and sending over the wire
    
    for (auto& [connId, messages] : outgoingBuffers_) {
        if (!messages.empty()) {
            // In a real implementation, we'd serialize each message to JSON
            // and send it over the WebSocket connection
            
            // For now, just clear the buffer
            messages.clear();
        }
    }
}

void WebSocketServer::handleConnection(ConnectionID connId) {
    // TODO: Handle new WebSocket connections
}

void WebSocketServer::handleDisconnection(ConnectionID connId) {
    unregisterConnection(connId);
}

ConnectionID WebSocketServer::generateConnectionId() {
    static ConnectionID nextId = 1;
    return nextId++;
}

uint64_t WebSocketServer::getCurrentTimestamp() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}
