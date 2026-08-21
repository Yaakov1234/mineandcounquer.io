#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <map>
#include "common/Types.h"

class CombatSystem {
public:
    CombatSystem();
    ~CombatSystem() = default;

    // Combat mechanics
    float calculateDamage(WeaponType weapon, float distance) const;
    float calculateArmorReduction(float damage, bool hasArmor) const;
    bool canHit(const glm::vec3& shooterPos, const glm::vec3& targetPos, WeaponType weapon) const;
    uint32_t getWeaponCost(WeaponType weapon) const;
    float getWeaponRange(WeaponType weapon) const;
    float getWeaponDamage(WeaponType weapon) const;
    float getWeaponFireRate(WeaponType weapon) const;
    
    // Destruction
    void damageStructure(Structure& structure, float damage);
    float getStructureDamage(float baseDamage) const;  // Reduce damage to structures
    
    // Weapon info
    std::string getWeaponName(WeaponType weapon) const;
    uint32_t getAmmoPerMagazine(WeaponType weapon) const;

private:
    std::map<WeaponType, WeaponConfig> weaponConfigs_;
    void initializeWeapons();
    
    static constexpr float STRUCTURE_DAMAGE_MULTIPLIER = 1.2f;  // Weapons do 20% more damage to structures
    static constexpr float ARMOR_DAMAGE_REDUCTION = 0.4f;  // Armor blocks 40% damage
};
