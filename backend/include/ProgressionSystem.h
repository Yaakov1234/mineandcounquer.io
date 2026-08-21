#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <set>
#include <map>
#include "common/Types.h"

enum class Achievement {
    FIRST_KILL,
    FIVE_KILLS,
    TEN_KILLS,
    FIFTY_KILLS,
    HUNDRED_KILLS,
    FIRST_STRUCTURE,
    CONQUER_1000_TERRITORY,
    CONQUER_10000_TERRITORY,
    RARE_GEM_FOUND,
    SURVIVE_10_MIN,
    SURVIVE_30_MIN,
    FIRST_VICTORY,
    DOUBLE_KILL,
    TRIPLE_KILL
};

struct AchievementInfo {
    Achievement id;
    std::string name;
    std::string description;
    uint32_t rewardGems;
};

class ProgressionSystem {
public:
    ProgressionSystem();
    ~ProgressionSystem() = default;

    // Skin management
    uint32_t getUnlockedSkin(const PlayerState& player);
    std::string getSkinName(uint32_t skinId) const;
    bool skinHasStartingWeapon(uint32_t skinId) const;
    bool skinHasArmor(uint32_t skinId) const;
    uint32_t getSkinRarity(uint32_t skinId) const;  // 0=common, 1=uncommon, 2=rare, 3=legendary

    // Achievement tracking
    std::vector<Achievement> checkAchievements(PlayerState& player);
    bool hasAchievement(const PlayerState& player, Achievement ach) const;
    AchievementInfo getAchievementInfo(Achievement ach) const;
    
    // Progression info
    uint32_t getProgressToNextKillMilestone(uint32_t currentKills) const;
    uint32_t getNextKillMilestone(uint32_t currentKills) const;
    std::string getPlayerTitle(uint32_t kills) const;
    
    // Rewards
    uint32_t getKillReward(uint32_t streak) const;
    uint32_t getDeathPenalty() const { return deathPenalty_; }

private:
    struct SkinConfig {
        std::string name;
        bool hasArmor;
        bool hasStartingWeapon;
        uint32_t unlockKills;
        uint32_t rarity;  // 0=common, 1=uncommon, 2=rare, 3=legendary
    };

    std::map<uint32_t, SkinConfig> skinConfigs_;
    std::map<Achievement, AchievementInfo> achievementConfigs_;
    std::set<Achievement> playerAchievements_;  // Cached achievements
    
    void initializeSkins();
    void initializeAchievements();
    
    uint32_t deathPenalty_ = 5;  // Gems lost on death
};
