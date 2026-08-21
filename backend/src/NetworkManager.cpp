#include "NetworkManager.h"
#include <iostream>

NetworkManager::NetworkManager(uint16_t port)
    : port_(port) {
}

NetworkManager::~NetworkManager() {
    shutdown();
}

bool NetworkManager::initialize() {
    std::cout << "Initializing network manager on port " << port_ << std::endl;
    running_ = true;
    // TODO: Initialize WebSocket server (using library like websocketpp or asio)
    return true;
}

void NetworkManager::shutdown() {
    std::cout << "Shutting down network manager" << std::endl;
    running_ = false;
    connectedPlayers_.clear();
}

void NetworkManager::update(float deltaTime) {
    messageBufferTime_ += deltaTime;
    
    // Flush messages periodically
    if (messageBufferTime_ >= messageFlushInterval_) {
        // TODO: Send queued messages to clients
        messageBufferTime_ = 0.0f;
    }
}

void NetworkManager::sendToPlayer(PlayerID playerId, const Message& msg) {
    // TODO: Send message to specific player via WebSocket
    std::cout << "Sending message to player " << playerId << std::endl;
}

void NetworkManager::broadcastToWorld(WorldID worldId, const Message& msg) {
    // TODO: Broadcast message to all players in world
    std::cout << "Broadcasting to world " << worldId << std::endl;
}

std::vector<Message> NetworkManager::getIncomingMessages() {
    std::vector<Message> messages;
    // TODO: Get queued incoming messages from WebSocket
    return messages;
}

void NetworkManager::sendWorldState(WorldID worldId, const json& state) {
    // TODO: Send serialized world state to all players in world
}

void NetworkManager::onPlayerConnected(PlayerID playerId) {
    connectedPlayers_.push_back(playerId);
    std::cout << "Player " << playerId << " connected" << std::endl;
}

void NetworkManager::onPlayerDisconnected(PlayerID playerId) {
    connectedPlayers_.erase(
        std::remove(connectedPlayers_.begin(), connectedPlayers_.end(), playerId),
        connectedPlayers_.end()
    );
    std::cout << "Player " << playerId << " disconnected" << std::endl;
}

bool NetworkManager::isPlayerConnected(PlayerID playerId) const {
    return std::find(connectedPlayers_.begin(), connectedPlayers_.end(), playerId) != connectedPlayers_.end();
}

json NetworkManager::serializePlayer(const PlayerState& player) {
    json j;
    j["id"] = player.id;
    j["username"] = player.username;
    j["position"] = {player.position.x, player.position.y, player.position.z};
    j["health"] = player.health;
    j["maxHealth"] = player.maxHealth;
    j["kills"] = player.kills;
    j["deaths"] = player.deaths;
    j["territory"] = player.territoryControlled;
    j["inMine"] = (player.currentZone == ZoneType::MINE);
    j["alive"] = player.isAlive;
    j["skinId"] = player.skinId;
    j["streak"] = player.streak;
    return j;
}

json NetworkManager::serializeStructure(const Structure& structure) {
    json j;
    j["id"] = structure.id;
    j["ownerId"] = structure.ownerId;
    j["position"] = {structure.position.x, structure.position.y, structure.position.z};
    j["health"] = structure.health;
    j["maxHealth"] = structure.maxHealth;
    j["destroyed"] = structure.isDestroyed;
    j["buildProgress"] = structure.buildProgress;
    return j;
}

json NetworkManager::serializeResource(const Resource& resource) {
    json j;
    j["id"] = resource.id;
    j["type"] = (resource.type == ResourceType::GEM ? "gem" : "rock");
    j["position"] = {resource.position.x, resource.position.y, resource.position.z};
    j["value"] = resource.value;
    j["collected"] = resource.collected;
    return j;
}

json NetworkManager::serializeInventory(const Inventory& inv) {
    json j;
    j["gems"] = inv.gems;
    j["rocks"] = inv.rocks;
    j["weapons"] = json::object();
    for (const auto& [weapon, count] : inv.weapons) {
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
