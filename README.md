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
cd backend
mkdir build
cd build
cmake ..
make
```

### Frontend Setup
```bash
cd frontend
npm install
npm start
```

## Development

See `docs/ARCHITECTURE.md` for detailed design documentation.
