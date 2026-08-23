# Mine and Conquer

An IO game where players mine resources, build fortresses, acquire weapons, and conquer territory.

## Project Structure

```
mineandcounquer.io/
├── backend/          # C++ server
├── frontend/         # Three.js web client
├── docs/             # Game design & documentation
└── README.md
```

## Tech Stack

- **Backend**: C++ (game logic, networking, state management)
- **Frontend**: Three.js (3D rendering, WebGL)
- **Communication**: WebSockets
- **Build**: CMake (C++), Node.js/Webpack (frontend)

## Game Overview

### Core Loop
1. **Mine** (Safe Zone) - Gather gems and rocks underground
2. **Surface** - Build fortress with materials
3. **Arm** - Buy weapons from dealer with gems
4. **Combat** - Defend territory, raid others, survive
5. **Respawn** - Die and join random room

### Key Features
- Multiple rooms (instances)
- Territory conquest system
- Skin/achievement progression
- Real-time multiplayer combat
- Mining economy

## Getting Started

### Backend Setup
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/mine_server 8080
```

The server accepts WebSocket connections at `ws://localhost:8080`. A browser client is not included yet.

### Minimal protocol

All client messages are JSON objects with a `type` and optional `payload`.

```json
{ "type": "join", "payload": { "username": "miner" } }
{ "type": "move", "payload": { "velocity": { "x": 10, "y": 0, "z": 0 } } }
{ "type": "exit_mine" }
```

After joining, the server assigns the player ID itself and returns `joined`. It emits `world_state` snapshots at 20 Hz. Supported actions are `move`, `exit_mine`, `enter_mine`, `buy_weapon`, `place_structure`, `fire_weapon`, and `ping`.

### Requirements

- CMake 3.24 or later
- A C++20 compiler
- Boost.System development headers

CMake downloads GLM, nlohmann/json, and websocketpp during configuration.

## Development

See `docs/ARCHITECTURE.md` for the intended full game design. The immediate next milestone is the Three.js client that connects to this server.

