#include "GameServer.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <map>
#include <thread>

namespace {
constexpr float kTickDuration = 1.0f / 60.0f;

json positionToJson(const glm::vec3& position) {
    return {{"x", position.x}, {"y", position.y}, {"z", position.z}};
}
}

GameServer::GameServer(uint16_t port) : network_(port) {}

GameServer::~GameServer() {
    stop();
    network_.shutdown();
}

bool GameServer::initialize() {
    world_ = std::make_unique<World>(1);
    if (!network_.initialize()) return false;
    running_ = true;
    return true;
}

void GameServer::run() {
    auto previous = std::chrono::steady_clock::now();
    float accumulator = 0.0f;
    while (running_) {
        const auto now = std::chrono::steady_clock::now();
        const float elapsed = std::min(0.25f, std::chrono::duration<float>(now - previous).count());
        previous = now;
        accumulator += elapsed;

        while (accumulator >= kTickDuration) {
            processMessages();
            world_->update(kTickDuration);
            network_.update(kTickDuration);
            stateBroadcastTimer_ += kTickDuration;
            if (stateBroadcastTimer_ >= stateBroadcastInterval_) {
                sendWorldState();
                stateBroadcastTimer_ = 0.0f;
            }
            accumulator -= kTickDuration;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    network_.shutdown();
}

void GameServer::stop() {
    running_ = false;
}

void GameServer::processMessages() {
    for (const Message& message : network_.getIncomingMessages()) handleMessage(message);
}

void GameServer::handleMessage(const Message& message) {
    switch (message.type) {
        case MessageType::PLAYER_JOIN: {
            const std::string username = message.payload.value("username", "Player" + std::to_string(message.playerId));
            if (username.empty() || username.size() > 24 || world_->addPlayer(message.playerId, username) == 0) {
                sendError(message.playerId, "Unable to join this world.");
                return;
            }
            network_.setPlayerWorld(message.playerId, world_->getId());
            const Player* player = world_->getPlayer(message.playerId);
            network_.sendToPlayer(message.playerId, {MessageType::PLAYER_JOINED, {
                {"playerId", message.playerId}, {"worldId", world_->getId()}, {"position", positionToJson(player->getPosition())}
            }});
            break;
        }
        case MessageType::PLAYER_DISCONNECT:
            world_->removePlayer(message.playerId);
            network_.onPlayerDisconnected(message.playerId);
            lastShotAt_.erase(message.playerId);
            break;
        case MessageType::PLAYER_MOVE: {
            Player* player = world_->getPlayer(message.playerId);
            glm::vec3 velocity;
            if (!player || !readVec3(message.payload.value("velocity", json{}), velocity)) {
                sendError(message.playerId, "A move message needs a velocity object with x, y, and z values.");
                return;
            }
            player->setVelocity(velocity);
            break;
        }
        case MessageType::PLAYER_EXIT_MINE:
            if (!world_->canExitMine(message.playerId)) sendError(message.playerId, "Collect at least one resource before leaving the mine.");
            else world_->movePlayerToSurface(message.playerId);
            break;
        case MessageType::PLAYER_ENTER_MINE:
            world_->movePlayerToMine(message.playerId);
            break;
        case MessageType::PLAYER_BUY_WEAPON: {
            bool valid = false;
            const WeaponType weapon = weaponFromJson(message.payload.value("weapon", json{}), valid);
            if (!valid || !world_->buyWeapon(message.playerId, weapon)) sendError(message.playerId, "Unable to buy that weapon.");
            break;
        }
        case MessageType::PLAYER_PLACE_STRUCTURE: {
            bool valid = false;
            const StructureType type = structureFromJson(message.payload.value("structure", json{}), valid);
            glm::vec3 position;
            if (!valid || !readVec3(message.payload.value("position", json{}), position) ||
                world_->addStructure(message.playerId, position, type) == 0) {
                sendError(message.playerId, "Unable to place that structure.");
            }
            break;
        }
        case MessageType::PLAYER_FIRE_WEAPON: {
            bool valid = false;
            const WeaponType weapon = weaponFromJson(message.payload.value("weapon", json{}), valid);
            glm::vec3 target;
            if (!valid || !readVec3(message.payload.value("target", json{}), target)) {
                sendError(message.playerId, "A fire message needs a valid weapon and target.");
                return;
            }
            const auto now = std::chrono::steady_clock::now();
            const float rate = world_->getWeaponFireRate(weapon);
            const auto last = lastShotAt_.find(message.playerId);
            if (rate <= 0.0f || (last != lastShotAt_.end() &&
                std::chrono::duration<float>(now - last->second).count() < 1.0f / rate)) {
                sendError(message.playerId, "Weapon is cooling down.");
                return;
            }
            lastShotAt_[message.playerId] = now;
            if (!world_->fireWeapon(message.playerId, target, weapon)) sendError(message.playerId, "That shot was invalid or missed.");
            break;
        }
        case MessageType::PING:
            network_.sendToPlayer(message.playerId, {MessageType::PONG, message.payload});
            break;
        default:
            break;
    }
}

void GameServer::sendWorldState() {
    json state;
    state["tick"] = world_->getWorldTickCount();
    state["players"] = json::array();
    state["structures"] = json::array();
    state["resources"] = json::array();
    for (const Player* player : world_->getAllPlayers()) state["players"].push_back(NetworkManager::serializePlayer(player->getState()));
    for (const auto& structure : world_->getAllStructures()) state["structures"].push_back(NetworkManager::serializeStructure(*structure));
    for (const auto& resource : world_->getAllResources()) state["resources"].push_back(NetworkManager::serializeResource(*resource));
    network_.sendWorldState(world_->getId(), state);
}

void GameServer::sendError(PlayerID playerId, const std::string& reason) {
    network_.sendToPlayer(playerId, {MessageType::ERROR_MSG, {{"message", reason}}});
}

bool GameServer::readVec3(const json& value, glm::vec3& result) {
    if (!value.is_object() || !value.contains("x") || !value.contains("y") || !value.contains("z")) return false;
    try {
        result = {value.at("x").get<float>(), value.at("y").get<float>(), value.at("z").get<float>()};
        return std::isfinite(result.x) && std::isfinite(result.y) && std::isfinite(result.z);
    } catch (const json::exception&) {
        return false;
    }
}

WeaponType GameServer::weaponFromJson(const json& value, bool& valid) {
    static const std::map<std::string, WeaponType> weapons = {
        {"pistol", WeaponType::PISTOL}, {"rifle", WeaponType::RIFLE}, {"shotgun", WeaponType::SHOTGUN},
        {"sniper", WeaponType::SNIPER}, {"rocket", WeaponType::ROCKET_LAUNCHER}
    };
    valid = value.is_string() && weapons.find(value.get<std::string>()) != weapons.end();
    return valid ? weapons.at(value.get<std::string>()) : WeaponType::PISTOL;
}

StructureType GameServer::structureFromJson(const json& value, bool& valid) {
    static const std::map<std::string, StructureType> structures = {
        {"wall", StructureType::WALL}, {"tower", StructureType::TOWER}, {"base", StructureType::BASE},
        {"storage", StructureType::STORAGE}, {"turret", StructureType::TURRET}
    };
    valid = value.is_string() && structures.find(value.get<std::string>()) != structures.end();
    return valid ? structures.at(value.get<std::string>()) : StructureType::WALL;
}

