cmake_minimum_required(VERSION 3.16)
project(MineAndConquer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(glm REQUIRED)
find_package(nlohmann_json REQUIRED)

# Try to find websocketpp, if not found, we'll use a simple implementation
find_package(websocketpp QUIET)

# Source files
set(BACKEND_SOURCES
    src/main.cpp
    src/GameServer.cpp
    src/World.cpp
    src/Player.cpp
    src/NetworkManager.cpp
    src/WebSocketServer.cpp
    src/MessageHandler.cpp
    src/MiningSystem.cpp
    src/BuildingSystem.cpp
    src/CombatSystem.cpp
    src/EconomySystem.cpp
    src/ProgressionSystem.cpp
    src/Physics.cpp
)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Create executable
add_executable(mineandconquer_server ${BACKEND_SOURCES})

# Link libraries
target_link_libraries(mineandconquer_server 
    glm::glm
    nlohmann_json::nlohmann_json
)

# Link websocketpp if available
if(websocketpp_FOUND)
    target_link_libraries(mineandconquer_server websocketpp::websocketpp)
    target_compile_definitions(mineandconquer_server PRIVATE USE_WEBSOCKETPP=1)
endif()

# Output directories
set_target_properties(mineandconquer_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
