#pragma once

#include <cstdint>
#include <map>
#include "common/Types.h"

class EconomySystem {
public:
    EconomySystem();
    ~EconomySystem() = default;

    // Dealer mechanics
    uint32_t getDealerEntryFee() const { return dealerEntryFee_; }
    uint32_t getWeaponPrice(WeaponType weapon) const;
    bool canAffordWeapon(const Inventory& inv, WeaponType weapon) const;
    bool canAffordDealerEntry(const Inventory& inv) const;

    // Gem trading
    uint32_t gemToRockConversion(uint32_t gems) const;
    uint32_t rockToGemConversion(uint32_t rocks) const;
    
    // Refund on structure destruction
    uint32_t getDestructionRefund(StructureType type, ResourceType resource) const;
    
    // Economic info
    uint32_t getTotalWealthInGems(const Inventory& inv) const;
    std::string formatCurrency(uint32_t gems) const;

private:
    uint32_t dealerEntryFee_ = 10;  // Cost in gems to enter dealer
    static constexpr float GEM_TO_ROCK_RATIO = 5.0f;  // 1 gem = 5 rocks
    static constexpr float REFUND_PERCENTAGE = 0.5f;  // 50% refund on destruction
};
