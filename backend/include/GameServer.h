#pragma once

#include <chrono>
#include <atomic>
#include <memory>
#include <unordered_map>

#include "NetworkManager.h"
#include "World.h"

class GameServer {
public:
    explicit GameServer(uint16_t port = 8080);
    ~GameServer();

    bool initialize();
    void run();
    void stop();
    bool isRunning() const { return running_.load(); }

private:
    void processMessages();
    void handleMessage(const Message& message);
    void sendWorldState();
    void sendError(PlayerID playerId, const std::string& reason);
    static bool readVec3(const json& value, glm::vec3& result);
    static WeaponType weaponFromJson(const json& value, bool& valid);
    static StructureType structureFromJson(const json& value, bool& valid);

    NetworkManager network_;
    std::unique_ptr<World> world_;
    std::atomic_bool running_ = false;
    float stateBroadcastTimer_ = 0.0f;
    float stateBroadcastInterval_ = 1.0f / 20.0f;
    std::unordered_map<PlayerID, std::chrono::steady_clock::time_point> lastShotAt_;
};

