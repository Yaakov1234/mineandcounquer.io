cmake_minimum_required(VERSION 3.24)

project(mineandconquer_server VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
find_package(Boost 1.74 REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    websocketpp
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
    GIT_TAG 0.8.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm nlohmann_json websocketpp)

set(GAME_CORE_SOURCES
    backend/src/BuildingSystem.cpp
    backend/src/CombatSystem.cpp
    backend/src/EconomySystem.cpp
    backend/src/GameServer.cpp
    backend/src/MiningSystem.cpp
    backend/src/NetworkManager.cpp
    backend/src/Physics.cpp
    backend/src/Player.cpp
    backend/src/ProgressionSystem.cpp
    backend/src/World.cpp
)

add_library(game_core ${GAME_CORE_SOURCES})
target_include_directories(game_core PUBLIC backend/include ${websocketpp_SOURCE_DIR})
target_link_libraries(game_core PUBLIC glm::glm nlohmann_json::nlohmann_json Boost::system Threads::Threads)
target_compile_options(game_core PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive-> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>)

add_executable(mine_server backend/src/main.cpp)
target_link_libraries(mine_server PRIVATE game_core)

include(CTest)
if(BUILD_TESTING)
    add_executable(world_tests tests/WorldTests.cpp)
    target_link_libraries(world_tests PRIVATE game_core)
    add_test(NAME world_tests COMMAND world_tests)
endif()

