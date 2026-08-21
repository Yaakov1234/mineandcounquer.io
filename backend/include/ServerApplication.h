#pragma once

#include <memory>
#include "GameServerWithNetwork.h"

class ServerApplication {
public:
    ServerApplication(uint16_t gamePort = 8080, uint16_t wsPort = 9090);
    ~ServerApplication() = default;

    bool initialize();
    void run();
    void shutdown();

private:
    std::unique_ptr<GameServerWithNetwork> server_;
    bool running_ = false;
    
    static constexpr float TARGET_TICK_RATE = 60.0f;
    static constexpr float FRAME_TIME = 1.0f / TARGET_TICK_RATE;
};
