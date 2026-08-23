#include "CombatSystem.h"
#include <map>
#include <iostream>

CombatSystem::CombatSystem() {
    initializeWeapons();
}

float CombatSystem::calculateDamage(WeaponType weapon, float distance) const {
    auto it = weaponConfigs_.find(weapon);
    if (it == weaponConfigs_.end()) {
        return 0.0f;
    }

    float baseDamage = it->second.baseDamage;
    float range = it->second.range;

    // Damage decreases with distance (falloff)
    if (distance > range) {
        return 0.0f;
    }
    
    float damageMultiplier = 1.0f - (distance / range) * 0.5f;  // 50-100% of base damage
    return baseDamage * damageMultiplier;
}

float CombatSystem::calculateArmorReduction(float damage, bool hasArmor) const {
    if (!hasArmor) {
        return damage;
    }
    return damage * (1.0f - ARMOR_DAMAGE_REDUCTION);
}

bool CombatSystem::canHit(const glm::vec3& shooterPos, const glm::vec3& targetPos, WeaponType weapon) const {
    float distance = glm::distance(shooterPos, targetPos);
    float range = getWeaponRange(weapon);
    return distance <= range;
}

uint32_t CombatSystem::getWeaponCost(WeaponType weapon) const {
    auto it = weaponConfigs_.find(weapon);
    return it != weaponConfigs_.end() ? it->second.cost : 0;
}

float CombatSystem::getWeaponRange(WeaponType weapon) const {
    auto it = weaponConfigs_.find(weapon);
    return it != weaponConfigs_.end() ? it->second.range : 0.0f;
}

float CombatSystem::getWeaponDamage(WeaponType weapon) const {
    auto it = weaponConfigs_.find(weapon);
    return it != weaponConfigs_.end() ? it->second.baseDamage : 0.0f;
}

float CombatSystem::getWeaponFireRate(WeaponType weapon) const {
    auto it = weaponConfigs_.find(weapon);
    return it != weaponConfigs_.end() ? it->second.fireRate : 0.0f;
}

void CombatSystem::damageStructure(Structure& structure, float damage) {
    structure.health -= damage;
    if (structure.health <= 0.0f) {
        structure.isDestroyed = true;
        std::cout << "Structure " << structure.id << " destroyed!" << std::endl;
    }
}

float CombatSystem::getStructureDamage(float baseDamage) const {
    return baseDamage * STRUCTURE_DAMAGE_MULTIPLIER;
}

std::string CombatSystem::getWeaponName(WeaponType weapon) const {
    static const std::map<WeaponType, std::string> names = {
        {WeaponType::PISTOL, "Pistol"},
        {WeaponType::RIFLE, "Rifle"},
        {WeaponType::SHOTGUN, "Shotgun"},
        {WeaponType::SNIPER, "Sniper Rifle"},
        {WeaponType::ROCKET_LAUNCHER, "Rocket Launcher"}
    };
    auto it = names.find(weapon);
    return it != names.end() ? it->second : "Unknown";
}

uint32_t CombatSystem::getAmmoPerMagazine(WeaponType weapon) const {
    static const std::map<WeaponType, uint32_t> ammo = {
        {WeaponType::PISTOL, 12},
        {WeaponType::RIFLE, 30},
        {WeaponType::SHOTGUN, 8},
        {WeaponType::SNIPER, 5},
        {WeaponType::ROCKET_LAUNCHER, 4}
    };
    auto it = ammo.find(weapon);
    return it != ammo.end() ? it->second : 0;
}

void CombatSystem::initializeWeapons() {
    weaponConfigs_[WeaponType::PISTOL] = {10.0f, 50.0f, 5.0f, 50, 0.8f};
    weaponConfigs_[WeaponType::RIFLE] = {25.0f, 150.0f, 10.0f, 150, 0.9f};
    weaponConfigs_[WeaponType::SHOTGUN] = {40.0f, 30.0f, 2.0f, 200, 0.7f};
    weaponConfigs_[WeaponType::SNIPER] = {60.0f, 300.0f, 1.0f, 300, 1.0f};
    weaponConfigs_[WeaponType::ROCKET_LAUNCHER] = {100.0f, 200.0f, 0.5f, 500, 0.6f};
}

