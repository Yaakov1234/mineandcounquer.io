#include "ServerApplication.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>

static ServerApplication* g_pApp = nullptr;

void signalHandler(int signal) {
    if (g_pApp) {
        std::cout << "\nReceived signal " << signal << ". Shutting down..." << std::endl;
        g_pApp->shutdown();
    }
}

ServerApplication::ServerApplication(uint16_t gamePort, uint16_t wsPort)
    : server_(std::make_unique<GameServerWithNetwork>(gamePort, wsPort)) {
    g_pApp = this;
}

bool ServerApplication::initialize() {
    std::cout << "Initializing Server Application..." << std::endl;
    
    // Register signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    return true;
}

void ServerApplication::run() {
    if (!server_) {
        std::cerr << "Server not initialized" << std::endl;
        return;
    }
    
    std::cout << "Starting server..." << std::endl;
    server_->start();
    running_ = true;
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    uint64_t frameCount = 0;
    auto fpsTime = std::chrono::high_resolution_clock::now();
    
    while (running_ && server_->isRunning()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Update server
        server_->update(elapsed);
        frameCount++;
        
        // Calculate and print FPS every second
        auto fpsDuration = std::chrono::duration<float>(currentTime - fpsTime).count();
        if (fpsDuration >= 1.0f) {
            std::cout << "FPS: " << frameCount << " | Players: " 
                      << server_->getWebSocketServer()->getConnectionCount() << std::endl;
            frameCount = 0;
            fpsTime = currentTime;
        }
        
        // Sleep to maintain tick rate
        auto frameDuration = std::chrono::duration<float>(FRAME_TIME);
        std::this_thread::sleep_until(currentTime + frameDuration);
    }
    
    std::cout << "Server stopped" << std::endl;
}

void ServerApplication::shutdown() {
    running_ = false;
    if (server_) {
        server_->stop();
    }
}
