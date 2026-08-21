#pragma once

#include <glm/glm.hpp>
#include <string>
#include <chrono>
#include "common/Types.h"

class Player {
public:
    Player(PlayerID id, const std::string& username, const glm::vec3& startPos);
    ~Player() = default;

    // Updates
    void update(float deltaTime);
    void setVelocity(const glm::vec3& vel);
    void applyDamage(float damage);
    void heal(float amount);
    void addResource(ResourceType type, uint32_t amount);
    bool removeResource(ResourceType type, uint32_t amount);
    void addWeapon(WeaponType weapon, uint32_t count = 1);
    bool removeWeapon(WeaponType weapon, uint32_t count = 1);
    
    // Zone transitions
    void enterZone(ZoneType zone) { state_.currentZone = zone; }
    ZoneType getCurrentZone() const { return state_.currentZone; }
    
    // Combat
    void recordKill() { state_.kills++; state_.streak++; }
    void recordDeath() { state_.deaths++; state_.streak = 0; }
    uint32_t getStreak() const { return state_.streak; }
    
    // Territory
    void addTerritory(uint64_t amount) { state_.territoryControlled += amount; }
    void removeTerritory(uint64_t amount) {
        state_.territoryControlled = state_.territoryControlled > amount ? 
            state_.territoryControlled - amount : 0;
    }
    
    // Skin & Progression
    void setSkinId(uint32_t skinId) { state_.skinId = skinId; }
    void setArmor(bool hasArmor) { state_.hasArmor = hasArmor; }
    bool hasArmor() const { return state_.hasArmor; }
    
    // Getters
    PlayerID getId() const { return state_.id; }
    const std::string& getUsername() const { return state_.username; }
    const glm::vec3& getPosition() const { return state_.position; }
    const glm::vec3& getVelocity() const { return state_.velocity; }
    float getHealth() const { return state_.health; }
    float getMaxHealth() const { return state_.maxHealth; }
    float getHealthPercent() const { return state_.health / state_.maxHealth; }
    bool isAlive() const { return state_.isAlive && state_.health > 0.0f; }
    bool isInMine() const { return state_.currentZone == ZoneType::MINE; }
    const Inventory& getInventory() const { return state_.inventory; }
    const PlayerState& getState() const { return state_; }
    uint32_t getKills() const { return state_.kills; }
    uint32_t getDeaths() const { return state_.deaths; }
    float getKDA() const { return state_.deaths > 0 ? 
        (float)state_.kills / state_.deaths : (float)state_.kills; }
    uint64_t getTerritoryControlled() const { return state_.territoryControlled; }
    uint32_t getSkinId() const { return state_.skinId; }

    // Setters
    void setPosition(const glm::vec3& pos) { state_.position = pos; }
    void setMoving(bool moving) { state_.isMoving = moving; }
    bool isMoving() const { return state_.isMoving; }

private:
    PlayerState state_;
    float friction_ = 0.95f;
    float maxSpeed_ = 50.0f;
    float acceleration_ = 100.0f;
};
