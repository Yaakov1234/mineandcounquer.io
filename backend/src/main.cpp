#include "GameServer.h"

#include <csignal>
#include <cstdlib>
#include <iostream>

namespace {
GameServer* server = nullptr;

void stopServer(int) {
    if (server) {
        server->stop();
    }
}
}

int main(int argc, char* argv[]) {
    uint16_t port = 8080;
    if (argc == 2) {
        const auto parsed = std::strtoul(argv[1], nullptr, 10);
        if (parsed == 0 || parsed > 65535) {
            std::cerr << "Usage: mine_server [port 1-65535]" << std::endl;
            return 1;
        }
        port = static_cast<uint16_t>(parsed);
    }

    GameServer gameServer(port);
    server = &gameServer;
    std::signal(SIGINT, stopServer);
    std::signal(SIGTERM, stopServer);

    if (!gameServer.initialize()) {
        return 1;
    }
    gameServer.run();
    return 0;
}

