#include <iostream>
#include "ServerApplication.h"

int main(int argc, char* argv[]) {
    std::cout << "="  << std::endl;
    std::cout << "Mine and Conquer - Game Server" << std::endl;
    std::cout << "Version 1.0.0" << std::endl;
    std::cout << "================" << std::endl << std::endl;

    uint16_t gamePort = 8080;
    uint16_t wsPort = 9090;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-p" || arg == "--port") {
            if (i + 1 < argc) {
                wsPort = std::stoi(argv[++i]);
            }
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -p, --port PORT   WebSocket server port (default: 9090)" << std::endl;
            std::cout << "  -h, --help        Show this help message" << std::endl;
            return 0;
        }
    }
    
    std::cout << "Starting server..." << std::endl;
    std::cout << "  Game Port: " << gamePort << std::endl;
    std::cout << "  WebSocket Port: " << wsPort << std::endl << std::endl;

    try {
        ServerApplication app(gamePort, wsPort);
        
        if (!app.initialize()) {
            std::cerr << "Failed to initialize application" << std::endl;
            return 1;
        }
        
        app.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\nServer shutdown complete." << std::endl;
    return 0;
}
