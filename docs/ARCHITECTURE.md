# Mine and Conquer - Architecture Document

## System Overview

```
┌─────────────────────────────────────────────────────────────┐
│            Web Browser (Client)                             │
│  ┌───────────────────────────────────────────────────────┐  │
│  │  Three.js 3D Renderer                               │  │
│  │  - Underground Mine (Safe Zone)                      │  │
│  │  - Surface World (Building/Combat Zone)              │  │
│  │  - UI/HUD Layer                                      │  │
│  └───────────────────────────────────────────────────────┘  │
│                    WebSocket                                │
└─────────────────────────────────────────────────────────────┘
           ↕ (Real-time State Updates)
┌─────────────────────────────────────────────────────────────┐
│          C++ Game Server                                    │
│  ┌───────────────────────────────────────────────────────┐  │
│  │  Game Engine                                         │  │
│  │  - Game Loop (60 Hz target)                          │  │
│  │  - Physics Simulation                                │  │
│  │  - Collision Detection                               │  │
│  │  - State Management                                  │  │
│  └───────────────────────────────────────────────────────┘  │
│  ┌───────────────────────────────────────────────────────┐  │
│  │  Game Systems                                        │  │
│  │  - Mining System                                     │  │
│  │  - Building/Territory System                         │  │
│  │  - Combat System                                     │  │
│  │  - Player Progression (Skins/Achievements)           │  │
│  │  - Economy (Gem/Rock Trading)                        │  │
│  └───────────────────────────────────────────────────────┘  │
│  ┌───────────────────────────────────────────────────────┐  │
│  │  Networking                                          │  │
│  │  - WebSocket Server                                  │  │
│  │  - Message Handling                                  │  │
│  │  - Room Management (Instances)                       │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
           ↕ (Persistence)
┌─────────────────────────────────────────────────────────────┐
│  Database (PostgreSQL/SQLite)                               │
│  - Player Data (Progress, Skins, Stats)                     │
│  - World State (Territory, Structures)                      │
│  - Session Data (Rooms, Active Players)                     │
└─────────────────────────────────────────────────────────────┘
```

## Backend Architecture (C++)

### Core Components

#### 1. Game Server (`GameServer.h/cpp`)
- Main server loop (fixed timestep: ~60 Hz)
- Room/instance management
- Player connection handling
- Update broadcasting

#### 2. Game World (`World.h/cpp`)
- Represents a single game room/instance
- Player management
- Territory/structure tracking
- Mine resources spawning
- Update propagation to clients

#### 3. Player (`Player.h/cpp`)
- Player state (position, health, inventory)
- Inventory (gems, rocks, weapons)
- Building permissions/territory
- Skin/achievement data
- Input handling

#### 4. Systems

**Mining System** (`MiningSystem.h/cpp`)
- Resource spawn logic
- Gathering mechanics
- Gem/rock rarity and distribution

**Building System** (`BuildingSystem.h/cpp`)
- Territory placement and validation
- Structure health and durability
- Collision detection with existing structures
- Territory ownership

**Combat System** (`CombatSystem.h/cpp`)
- Weapon mechanics
- Damage calculation
- Hit detection
- Structure destruction
- Kill tracking

**Economy System** (`EconomySystem.h/cpp`)
- Weapon shop pricing
- Dealer gem trading
- Player wallet management

**Progression System** (`ProgressionSystem.h/cpp`)
- Skin unlocks
- Achievement tracking
- Stat recording (kills, deaths, territory, etc.)

#### 5. Networking (`NetworkManager.h/cpp`)
- WebSocket server integration
- Message serialization/deserialization
- Player input processing
- State delta transmission

#### 6. Physics (`Physics.h/cpp`)
- Collision detection (AABB for simplicity, can upgrade to spatial partitioning)
- Movement validation
- Environmental interactions

### Message Protocol

**Client → Server**
```json
{
  "type": "move",
  "direction": { "x": 1, "y": 0, "z": 0 },
  "position": { "x": 10, "y": 5, "z": 20 }
}
```

**Server → Client**
```json
{
  "type": "state_update",
  "tick": 12345,
  "players": [ { "id": 1, "x": 10, "y": 5, "z": 20, "health": 100 } ],
  "structures": [ { "id": 1, "owner": 1, "type": "wall", "health": 50 } ],
  "resources": [ { "id": 1, "type": "gem", "x": 15, "y": 5, "z": 25 } ]
}
```

## Frontend Architecture (Three.js)

### Core Components

#### 1. Scene Manager (`SceneManager.js`)
- Three.js scene setup
- Camera management
- Lighting and environment
- Underground mine vs. surface rendering

#### 2. Renderer (`Renderer.js`)
- 3D mesh generation (players, structures, resources)
- Animation loop
- Update synchronization with server state

#### 3. Input Handler (`InputHandler.js`)
- Keyboard/mouse input capture
- Player movement commands
- UI interaction
- Sends input to server

#### 4. Network Client (`NetworkClient.js`)
- WebSocket connection management
- Message sending/receiving
- State reconciliation

#### 5. UI (`UI.js`)
- HUD (health, inventory, current gems/rocks)
- Menu system (spawn screen, dealer shop, settings)
- Achievement/skin display
- Territory info overlay

#### 6. Asset Manager (`AssetManager.js`)
- Texture/model loading
- Skin variants
- Weapon visuals

## Data Models

### Player
```cpp
struct Player {
    uint32_t id;
    std::string username;
    glm::vec3 position;
    float health;
    uint32_t kills;
    uint32_t deaths;
    Inventory inventory;
    uint32_t skinId;
    uint64_t territoryControlled;
    bool isInMine;
};
```

### Structure
```cpp
struct Structure {
    uint32_t id;
    uint32_t ownerId;  // Player who built it
    glm::vec3 position;
    StructureType type;  // wall, tower, base, etc.
    float health;
    float maxHealth;
};
```

### Resource
```cpp
struct Resource {
    uint32_t id;
    ResourceType type;  // gem, rock
    glm::vec3 position;
    uint32_t value;  // Gem rarity or rock count
};
```

## Game Flow

1. **Player Joins**
   - Client connects via WebSocket
   - Server assigns player to random room
   - Player spawns in mine (safe zone)
   - Server sends initial world state

2. **Mining Phase**
   - Player moves through mine
   - Collects resources (gems, rocks)
   - Server updates inventory

3. **Exit Mine**
   - Player moves to surface
   - Server validates safe spawn location
   - Player can now build

4. **Building Phase**
   - Player places structures (costs resources)
   - Server validates placement
   - Structures are visible to all players

5. **Combat Phase**
   - Player goes to dealer, buys weapons
   - Player engages in combat
   - Server resolves damage/collisions
   - Territory is conquered/destroyed

6. **Death & Respawn**
   - Player health → 0
   - Player removed from world
   - Player assigned to new random room
   - Respawn in mine

## Performance Considerations

- **Tick Rate**: 60 Hz server, variable client refresh
- **State Compression**: Send only delta updates
- **Spatial Partitioning**: Divide world into zones (future optimization)
- **Physics**: Simplified AABB collision for performance
- **Rendering**: LOD for distant structures; frustum culling

## Security

- Client-side validation only for responsiveness
- Server authoritative on all game logic
- Cheat prevention: Position/damage validation
- Rate limiting on critical actions
