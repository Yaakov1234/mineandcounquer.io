#include "EconomySystem.h"
#include <map>

EconomySystem::EconomySystem() = default;

uint32_t EconomySystem::getWeaponPrice(WeaponType weapon) const {
    static const std::map<WeaponType, uint32_t> prices = {
        {WeaponType::PISTOL, 50},
        {WeaponType::RIFLE, 150},
        {WeaponType::SHOTGUN, 200},
        {WeaponType::SNIPER, 300},
        {WeaponType::ROCKET_LAUNCHER, 500}
    };

    auto it = prices.find(weapon);
    return it != prices.end() ? it->second : 0;
}

bool EconomySystem::canAffordWeapon(const Inventory& inv, WeaponType weapon) const {
    return inv.gems >= getWeaponPrice(weapon);
}

bool EconomySystem::canAffordDealerEntry(const Inventory& inv) const {
    return inv.gems >= dealerEntryFee_;
}

uint32_t EconomySystem::gemToRockConversion(uint32_t gems) const {
    return static_cast<uint32_t>(gems * GEM_TO_ROCK_RATIO);
}

uint32_t EconomySystem::rockToGemConversion(uint32_t rocks) const {
    return rocks / static_cast<uint32_t>(GEM_TO_ROCK_RATIO);
}

uint32_t EconomySystem::getDestructionRefund(StructureType type, ResourceType resource) const {
    // Return 50% of the original cost
    // This would need access to BuildingSystem for full cost info
    uint32_t refund = 0;
    
    switch (type) {
        case StructureType::WALL:
            refund = resource == ResourceType::GEM ? 5 : 10;
            break;
        case StructureType::TOWER:
            refund = resource == ResourceType::GEM ? 15 : 20;
            break;
        case StructureType::BASE:
            refund = resource == ResourceType::GEM ? 25 : 50;
            break;
        case StructureType::STORAGE:
            refund = resource == ResourceType::GEM ? 10 : 25;
            break;
        case StructureType::TURRET:
            refund = resource == ResourceType::GEM ? 20 : 15;
            break;
        default:
            break;
    }
    
    return static_cast<uint32_t>(refund * REFUND_PERCENTAGE);
}

uint32_t EconomySystem::getTotalWealthInGems(const Inventory& inv) const {
    uint32_t totalGems = inv.gems;
    totalGems += rockToGemConversion(inv.rocks);
    return totalGems;
}

std::string EconomySystem::formatCurrency(uint32_t gems) const {
    if (gems < 1000) {
        return std::to_string(gems) + "G";
    } else if (gems < 1000000) {
        return std::to_string(gems / 1000) + "." + std::to_string((gems % 1000) / 100) + "KG";
    } else {
        return std::to_string(gems / 1000000) + "MG";
    }
}
