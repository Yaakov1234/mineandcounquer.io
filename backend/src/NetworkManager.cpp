#include "NetworkManager.h"

#include <algorithm>
#include <atomic>
#include <iostream>
#include <map>
#include <thread>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace {
using WebSocketServer = websocketpp::server<websocketpp::config::asio>;
using ConnectionHandle = websocketpp::connection_hdl;

const char* messageName(MessageType type) {
    switch (type) {
        case MessageType::PLAYER_JOIN: return "join";
        case MessageType::PLAYER_MOVE: return "move";
        case MessageType::PLAYER_MINE: return "mine";
        case MessageType::PLAYER_FIRE_WEAPON: return "fire_weapon";
        case MessageType::PLAYER_PLACE_STRUCTURE: return "place_structure";
        case MessageType::PLAYER_EXIT_MINE: return "exit_mine";
        case MessageType::PLAYER_ENTER_MINE: return "enter_mine";
        case MessageType::PLAYER_BUY_WEAPON: return "buy_weapon";
        case MessageType::PLAYER_DISCONNECT: return "disconnect";
        case MessageType::PING: return "ping";
        case MessageType::PLAYER_JOINED: return "joined";
        case MessageType::WORLD_STATE: return "world_state";
        case MessageType::PLAYER_UPDATE: return "player_update";
        case MessageType::STRUCTURE_UPDATE: return "structure_update";
        case MessageType::RESOURCE_UPDATE: return "resource_update";
        case MessageType::PLAYER_DIED: return "player_died";
        case MessageType::PLAYER_KILLED: return "player_killed";
        case MessageType::STRUCTURE_DESTROYED: return "structure_destroyed";
        case MessageType::ACHIEVEMENT_UNLOCKED: return "achievement_unlocked";
        case MessageType::PONG: return "pong";
        case MessageType::ERROR_MSG: return "error";
    }
    return "error";
}

bool messageTypeFromName(const std::string& name, MessageType& type) {
    static const std::map<std::string, MessageType> types = {
        {"join", MessageType::PLAYER_JOIN}, {"move", MessageType::PLAYER_MOVE},
        {"mine", MessageType::PLAYER_MINE}, {"fire_weapon", MessageType::PLAYER_FIRE_WEAPON},
        {"place_structure", MessageType::PLAYER_PLACE_STRUCTURE}, {"exit_mine", MessageType::PLAYER_EXIT_MINE},
        {"enter_mine", MessageType::PLAYER_ENTER_MINE}, {"buy_weapon", MessageType::PLAYER_BUY_WEAPON},
        {"ping", MessageType::PING}
    };
    const auto found = types.find(name);
    if (found == types.end()) return false;
    type = found->second;
    return true;
}
}

struct NetworkManager::Impl {
    WebSocketServer server;
    std::thread worker;
    std::map<ConnectionHandle, PlayerID, std::owner_less<ConnectionHandle>> playerByConnection;
    std::unordered_map<PlayerID, ConnectionHandle> connectionByPlayer;
    std::unordered_map<PlayerID, WorldID> worldByPlayer;
    std::atomic<PlayerID> nextPlayerId{1};
    std::mutex connectionsMutex;
};

NetworkManager::NetworkManager(uint16_t port)
    : port_(port), impl_(std::make_unique<Impl>()) {
}

NetworkManager::~NetworkManager() {
    shutdown();
}

bool NetworkManager::initialize() {
    try {
        impl_->server.clear_access_channels(websocketpp::log::alevel::all);
        impl_->server.init_asio();
        impl_->server.set_reuse_addr(true);
        impl_->server.set_open_handler([](ConnectionHandle) {});
        impl_->server.set_close_handler([this](ConnectionHandle handle) {
            PlayerID playerId = 0;
            {
                std::lock_guard lock(impl_->connectionsMutex);
                const auto found = impl_->playerByConnection.find(handle);
                if (found != impl_->playerByConnection.end()) {
                    playerId = found->second;
                    impl_->connectionByPlayer.erase(playerId);
                    impl_->worldByPlayer.erase(playerId);
                    impl_->playerByConnection.erase(found);
                }
            }
            if (playerId != 0) enqueue({MessageType::PLAYER_DISCONNECT, json::object(), playerId});
        });
        impl_->server.set_message_handler([this](ConnectionHandle handle, WebSocketServer::message_ptr frame) {
            try {
                const json received = json::parse(frame->get_payload());
                const std::string name = received.value("type", "");
                const json payload = received.value("payload", json::object());
                MessageType type;
                if (!messageTypeFromName(name, type)) return;

                PlayerID playerId = 0;
                {
                    std::lock_guard lock(impl_->connectionsMutex);
                    const auto existing = impl_->playerByConnection.find(handle);
                    if (type == MessageType::PLAYER_JOIN) {
                        if (existing != impl_->playerByConnection.end()) return;
                        playerId = impl_->nextPlayerId++;
                        impl_->playerByConnection.emplace(handle, playerId);
                        impl_->connectionByPlayer.emplace(playerId, handle);
                    } else if (existing != impl_->playerByConnection.end()) {
                        playerId = existing->second;
                    }
                }
                if (playerId != 0) enqueue({type, payload, playerId});
            } catch (const std::exception&) {
                // Invalid frames are ignored; the game server remains available to other players.
            }
        });
        impl_->server.listen(port_);
        impl_->server.start_accept();
        running_ = true;
        impl_->worker = std::thread([this] { impl_->server.run(); });
        std::cout << "WebSocket server listening on port " << port_ << std::endl;
        return true;
    } catch (const std::exception& error) {
        std::cerr << "Unable to start WebSocket server: " << error.what() << std::endl;
        return false;
    }
}

void NetworkManager::shutdown() {
    if (!running_) return;
    running_ = false;
    websocketpp::lib::error_code error;
    impl_->server.stop_listening(error);
    impl_->server.stop();
    if (impl_->worker.joinable()) impl_->worker.join();
    std::lock_guard lock(impl_->connectionsMutex);
    impl_->playerByConnection.clear();
    impl_->connectionByPlayer.clear();
    impl_->worldByPlayer.clear();
}

void NetworkManager::update(float deltaTime) {
    messageBufferTime_ += deltaTime;
    
    if (messageBufferTime_ >= messageFlushInterval_) {
        messageBufferTime_ = 0.0f;
    }
}

void NetworkManager::sendToPlayer(PlayerID playerId, const Message& msg) {
    sendJson(playerId, {{"type", messageName(msg.type)}, {"payload", msg.payload}});
}

void NetworkManager::broadcastToWorld(WorldID worldId, const Message& msg) {
    std::vector<PlayerID> recipients;
    {
        std::lock_guard lock(impl_->connectionsMutex);
        for (const auto& [playerId, playerWorldId] : impl_->worldByPlayer) {
            if (playerWorldId == worldId) recipients.push_back(playerId);
        }
    }
    for (const PlayerID playerId : recipients) sendToPlayer(playerId, msg);
}

std::vector<Message> NetworkManager::getIncomingMessages() {
    std::vector<Message> messages;
    std::lock_guard lock(mutex_);
    while (!incomingMessages_.empty()) {
        messages.push_back(std::move(incomingMessages_.front()));
        incomingMessages_.pop();
    }
    return messages;
}

void NetworkManager::sendWorldState(WorldID worldId, const json& state) {
    broadcastToWorld(worldId, {MessageType::WORLD_STATE, state});
}

void NetworkManager::onPlayerConnected(PlayerID playerId) {
    (void)playerId;
}

void NetworkManager::onPlayerDisconnected(PlayerID playerId) {
    std::lock_guard lock(impl_->connectionsMutex);
    impl_->worldByPlayer.erase(playerId);
}

bool NetworkManager::isPlayerConnected(PlayerID playerId) const {
    std::lock_guard lock(impl_->connectionsMutex);
    return impl_->connectionByPlayer.contains(playerId);
}

void NetworkManager::setPlayerWorld(PlayerID playerId, WorldID worldId) {
    std::lock_guard lock(impl_->connectionsMutex);
    if (impl_->connectionByPlayer.contains(playerId)) impl_->worldByPlayer[playerId] = worldId;
}

void NetworkManager::enqueue(Message message) {
    std::lock_guard lock(mutex_);
    incomingMessages_.push(std::move(message));
}

void NetworkManager::sendJson(PlayerID playerId, const json& message) {
    ConnectionHandle handle;
    {
        std::lock_guard lock(impl_->connectionsMutex);
        const auto found = impl_->connectionByPlayer.find(playerId);
        if (found == impl_->connectionByPlayer.end()) return;
        handle = found->second;
    }
    websocketpp::lib::error_code error;
    impl_->server.send(handle, message.dump(), websocketpp::frame::opcode::text, error);
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

