#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>

using PlayerID = uint32_t;
using StructureID = uint32_t;
using ResourceID = uint32_t;
using WorldID = uint32_t;

// Resource types
enum class ResourceType {
    GEM,
    ROCK
};

// Structure types
enum class StructureType {
    WALL,
    TOWER,
    BASE,
    STORAGE,
    TURRET
};

// Weapon types
enum class WeaponType {
    PISTOL,
    RIFLE,
    SHOTGUN,
    SNIPER,
    ROCKET_LAUNCHER
};

// Zone types
enum class ZoneType {
    MINE,
    SURFACE
};

// Inventory structure
struct Inventory {
    uint32_t gems = 0;
    uint32_t rocks = 0;
    std::map<WeaponType, uint32_t> weapons;  // weapon type -> count

    uint32_t getTotalGems() const { return gems; }
    uint32_t getTotalRocks() const { return rocks; }
    uint32_t getWeaponCount(WeaponType type) const {
        auto it = weapons.find(type);
        return it != weapons.end() ? it->second : 0;
    }
    bool hasWeapon(WeaponType type) const { return getWeaponCount(type) > 0; }
};

// Resource entity
struct Resource {
    ResourceID id;
    ResourceType type;
    glm::vec3 position;
    uint32_t value;  // gem rarity or rock amount
    bool collected = false;
    float respawnTimer = 0.0f;  // Time until resource respawns
    float maxRespawnTime = 30.0f;  // 30 seconds to respawn
};

// Structure entity
struct Structure {
    StructureID id;
    PlayerID ownerId;
    glm::vec3 position;
    StructureType type;
    float health;
    float maxHealth;
    bool isDestroyed = false;
    float buildProgress = 1.0f;  // 0.0 = building, 1.0 = complete
    uint64_t buildTimeRemaining = 0;  // milliseconds
};

// Player state
struct PlayerState {
    PlayerID id;
    std::string username;
    glm::vec3 position;
    glm::vec3 velocity;
    float health;
    float maxHealth;
    uint32_t kills = 0;
    uint32_t deaths = 0;
    Inventory inventory;
    uint32_t skinId = 0;  // Default skin
    uint64_t territoryControlled = 0;
    ZoneType currentZone = ZoneType::MINE;  // Starts in mine
    bool isAlive = true;
    bool isMoving = false;
    float lastActionTime = 0.0f;  // Timestamp of last action
    uint32_t streak = 0;  // Kill streak
    bool hasArmor = false;  // Armor from premium skins
};

// Weapon configuration
struct WeaponConfig {
    float baseDamage;
    float range;
    float fireRate;  // shots per second
    uint32_t cost;  // gem cost
    float armorPenetration = 0.8f;  // 0-1, how much armor blocks
};

// Structure configuration
struct StructureConfig {
    uint32_t gemCost;
    uint32_t rockCost;
    float health;
    float radius;  // Territory radius
    uint64_t buildTimeMs;  // Build time in milliseconds
};
