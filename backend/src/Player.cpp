#include "Player.h"
#include <algorithm>
#include <glm/glm.hpp>

Player::Player(PlayerID id, const std::string& username, const glm::vec3& startPos)
    : state_{id, username, startPos, glm::vec3(0), 100.0f, 100.0f} {
}

void Player::update(float deltaTime) {
    if (!state_.isAlive) return;

    // Apply friction to velocity
    state_.velocity *= friction_;

    // Clamp speed
    float speed = glm::length(state_.velocity);
    if (speed > maxSpeed_) {
        state_.velocity = glm::normalize(state_.velocity) * maxSpeed_;
    }

    // Update position
    state_.position += state_.velocity * deltaTime;
    
    // Update last action time
    state_.lastActionTime = deltaTime;
}

void Player::setVelocity(const glm::vec3& vel) {
    state_.velocity = vel;
    state_.isMoving = glm::length(vel) > 0.1f;
}

void Player::applyDamage(float damage) {
    state_.health = std::max(0.0f, state_.health - damage);
    if (state_.health <= 0.0f) {
        state_.isAlive = false;
    }
}

void Player::heal(float amount) {
    state_.health = std::min(state_.maxHealth, state_.health + amount);
    if (state_.health > 0.0f) {
        state_.isAlive = true;
    }
}

void Player::addResource(ResourceType type, uint32_t amount) {
    if (type == ResourceType::GEM) {
        state_.inventory.gems += amount;
    } else if (type == ResourceType::ROCK) {
        state_.inventory.rocks += amount;
    }
}

bool Player::removeResource(ResourceType type, uint32_t amount) {
    if (type == ResourceType::GEM) {
        if (state_.inventory.gems >= amount) {
            state_.inventory.gems -= amount;
            return true;
        }
    } else if (type == ResourceType::ROCK) {
        if (state_.inventory.rocks >= amount) {
            state_.inventory.rocks -= amount;
            return true;
        }
    }
    return false;
}

void Player::addWeapon(WeaponType weapon, uint32_t count) {
    state_.inventory.weapons[weapon] += count;
}

bool Player::removeWeapon(WeaponType weapon, uint32_t count) {
    auto it = state_.inventory.weapons.find(weapon);
    if (it != state_.inventory.weapons.end() && it->second >= count) {
        it->second -= count;
        if (it->second == 0) {
            state_.inventory.weapons.erase(it);
        }
        return true;
    }
    return false;
}
