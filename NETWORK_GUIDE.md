# Build Instructions

## Prerequisites

### Linux/Mac
```bash
brew install cmake glm nlohmann-json
```

### Windows (vcpkg)
```bash
vcpkg install glm:x64-windows nlohmann-json:x64-windows
```

## Build Backend

```bash
cd backend
mkdir build
cd build
cmake ..
make -j4
```

## Run Server

```bash
./bin/mineandconquer_server --port 9090
```

## WebSocket Connection

Clients connect to: `ws://localhost:9090`

### Example Client Message
```json
{
  "type": 0,
  "payload": {
    "username": "Player1"
  }
}
```

## Architecture

```
┌─────────────────────┐
│  Web Browser Client │
│   (Three.js)        │
└──────────┬──────────┘
           │ WebSocket
           │ (ws://localhost:9090)
           ▼
┌─────────────────────────────────────┐
│   WebSocket Server (C++)            │
│  ┌──────────────────────────────┐   │
│  │  Message Handler             │   │
│  │  - Player Actions            │   │
│  │  - World State Sync          │   │
│  └──────────────────────────────┘   │
└──────────┬──────────────────────────┘
           │
           ▼
┌─────────────────────────────────────┐
│   Game Server (60 Hz Loop)          │
│  ┌──────────────────────────────┐   │
│  │  Game Worlds                 │   │
│  │  ┌──────────────────────┐    │   │
│  │  │  World 1 (Room)      │    │   │
│  │  │  - Players           │    │   │
│  │  │  - Structures        │    │   │
│  │  │  - Resources         │    │   │
│  │  │  - Physics           │    │   │
│  │  └──────────────────────┘    │   │
│  │  ┌──────────────────────┐    │   │
│  │  │  World 2 (Room)      │    │   │
│  │  │  ...                 │    │   │
│  │  └──────────────────────┘    │   │
│  └──────────────────────────────┘   │
└─────────────────────────────────────┘
```

## Message Protocol

### Client → Server Messages

**PLAYER_JOIN (0)**
```json
{
  "type": 0,
  "payload": {
    "username": "PlayerName"
  }
}
```

**PLAYER_MOVE (1)**
```json
{
  "type": 1,
  "payload": {
    "velocity": {
      "x": 10.0,
      "y": 0.0,
      "z": 5.0
    }
  }
}
```

**PLAYER_FIRE_WEAPON (3)**
```json
{
  "type": 3,
  "payload": {
    "weapon": 0,
    "target": {
      "x": 50.0,
      "y": 10.0,
      "z": 30.0
    }
  }
}
```

**PLAYER_PLACE_STRUCTURE (4)**
```json
{
  "type": 4,
  "payload": {
    "type": 0,
    "position": {
      "x": 100.0,
      "y": 0.0,
      "z": 150.0
    }
  }
}
```

### Server → Client Messages

**WORLD_STATE (8)**
```json
{
  "type": 8,
  "payload": {
    "tickCount": 12345,
    "players": [...],
    "structures": [...],
    "resources": [...]
  }
}
```

**PLAYER_UPDATE (9)**
```json
{
  "type": 9,
  "payload": {
    "id": 1,
    "position": {"x": 10, "y": 5, "z": 20},
    "health": 85,
    "kills": 5
  }
}
```

**SYNC_TICK (19)**
```json
{
  "type": 19,
  "payload": {
    "tick": 12345
  }
}
```
