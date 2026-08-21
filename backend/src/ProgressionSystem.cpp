#include "ProgressionSystem.h"
#include <map>
#include <iostream>

ProgressionSystem::ProgressionSystem() {
    initializeSkins();
    initializeAchievements();
}

uint32_t ProgressionSystem::getUnlockedSkin(const PlayerState& player) {
    // Default skin for everyone
    uint32_t skin = 0;

    // Unlock premium skins based on kill milestones
    if (player.kills >= 10) skin = 1;      // Silver skin at 10 kills
    if (player.kills >= 50) skin = 2;      // Gold skin at 50 kills
    if (player.kills >= 100) skin = 3;     // Diamond skin at 100 kills

    return skin;
}

std::string ProgressionSystem::getSkinName(uint32_t skinId) const {
    auto it = skinConfigs_.find(skinId);
    if (it != skinConfigs_.end()) {
        return it->second.name;
    }
    return "Unknown";
}

bool ProgressionSystem::skinHasStartingWeapon(uint32_t skinId) const {
    auto it = skinConfigs_.find(skinId);
    return it != skinConfigs_.end() && it->second.hasStartingWeapon;
}

bool ProgressionSystem::skinHasArmor(uint32_t skinId) const {
    auto it = skinConfigs_.find(skinId);
    return it != skinConfigs_.end() && it->second.hasArmor;
}

uint32_t ProgressionSystem::getSkinRarity(uint32_t skinId) const {
    auto it = skinConfigs_.find(skinId);
    return it != skinConfigs_.end() ? it->second.rarity : 0;
}

std::vector<Achievement> ProgressionSystem::checkAchievements(PlayerState& player) {
    std::vector<Achievement> newAchievements;

    // Check for kill achievements
    if (player.kills >= 1 && !hasAchievement(player, Achievement::FIRST_KILL)) {
        newAchievements.push_back(Achievement::FIRST_KILL);
        std::cout << "Achievement: First Kill!" << std::endl;
    }
    if (player.kills >= 5 && !hasAchievement(player, Achievement::FIVE_KILLS)) {
        newAchievements.push_back(Achievement::FIVE_KILLS);
    }
    if (player.kills >= 10 && !hasAchievement(player, Achievement::TEN_KILLS)) {
        newAchievements.push_back(Achievement::TEN_KILLS);
    }
    if (player.kills >= 50 && !hasAchievement(player, Achievement::FIFTY_KILLS)) {
        newAchievements.push_back(Achievement::FIFTY_KILLS);
    }
    if (player.kills >= 100 && !hasAchievement(player, Achievement::HUNDRED_KILLS)) {
        newAchievements.push_back(Achievement::HUNDRED_KILLS);
    }

    // Check streak achievements
    if (player.streak >= 2 && !hasAchievement(player, Achievement::DOUBLE_KILL)) {
        newAchievements.push_back(Achievement::DOUBLE_KILL);
    }
    if (player.streak >= 3 && !hasAchievement(player, Achievement::TRIPLE_KILL)) {
        newAchievements.push_back(Achievement::TRIPLE_KILL);
    }

    return newAchievements;
}

bool ProgressionSystem::hasAchievement(const PlayerState& player, Achievement ach) const {
    return playerAchievements_.find(ach) != playerAchievements_.end();
}

AchievementInfo ProgressionSystem::getAchievementInfo(Achievement ach) const {
    auto it = achievementConfigs_.find(ach);
    if (it != achievementConfigs_.end()) {
        return it->second;
    }
    return {ach, "Unknown", "Unknown achievement", 0};
}

uint32_t ProgressionSystem::getProgressToNextKillMilestone(uint32_t currentKills) const {
    uint32_t nextMilestone = getNextKillMilestone(currentKills);
    return nextMilestone - currentKills;
}

uint32_t ProgressionSystem::getNextKillMilestone(uint32_t currentKills) const {
    if (currentKills < 5) return 5;
    if (currentKills < 10) return 10;
    if (currentKills < 50) return 50;
    if (currentKills < 100) return 100;
    return currentKills + 100;  // Every 100 kills after 100
}

std::string ProgressionSystem::getPlayerTitle(uint32_t kills) const {
    if (kills < 5) return "Novice";
    if (kills < 10) return "Fighter";
    if (kills < 50) return "Warrior";
    if (kills < 100) return "Legendary";
    return "Unstoppable";
}

uint32_t ProgressionSystem::getKillReward(uint32_t streak) const {
    // Bonus gems for kill streaks
    if (streak <= 1) return 0;
    if (streak == 2) return 10;    // Double kill
    if (streak == 3) return 25;    // Triple kill
    if (streak >= 4) return 50;    // Quad+ kill
    return 0;
}

void ProgressionSystem::initializeSkins() {
    skinConfigs_[0] = {"Default", false, false, 0, 0};
    skinConfigs_[1] = {"Silver", false, false, 10, 1};
    skinConfigs_[2] = {"Gold", false, true, 50, 2};
    skinConfigs_[3] = {"Diamond", true, true, 100, 3};
}

void ProgressionSystem::initializeAchievements() {
    achievementConfigs_[Achievement::FIRST_KILL] = {Achievement::FIRST_KILL, "First Blood", "Get your first kill", 50};
    achievementConfigs_[Achievement::FIVE_KILLS] = {Achievement::FIVE_KILLS, "5 Kills", "Earn 5 kills", 100};
    achievementConfigs_[Achievement::TEN_KILLS] = {Achievement::TEN_KILLS, "10 Kills", "Earn 10 kills", 200};
    achievementConfigs_[Achievement::FIFTY_KILLS] = {Achievement::FIFTY_KILLS, "50 Kills", "Earn 50 kills", 500};
    achievementConfigs_[Achievement::HUNDRED_KILLS] = {Achievement::HUNDRED_KILLS, "100 Kills", "Earn 100 kills", 1000};
    achievementConfigs_[Achievement::FIRST_STRUCTURE] = {Achievement::FIRST_STRUCTURE, "Builder", "Place your first structure", 100};
    achievementConfigs_[Achievement::CONQUER_1000_TERRITORY] = {Achievement::CONQUER_1000_TERRITORY, "Conqueror", "Control 1000 territory", 300};
    achievementConfigs_[Achievement::CONQUER_10000_TERRITORY] = {Achievement::CONQUER_10000_TERRITORY, "Empire", "Control 10000 territory", 1000};
    achievementConfigs_[Achievement::RARE_GEM_FOUND] = {Achievement::RARE_GEM_FOUND, "Gem Hunter", "Find a rare gem (rarity 5)", 200};
    achievementConfigs_[Achievement::SURVIVE_10_MIN] = {Achievement::SURVIVE_10_MIN, "Survivor", "Survive 10 minutes", 150};
    achievementConfigs_[Achievement::SURVIVE_30_MIN] = {Achievement::SURVIVE_30_MIN, "Veteran", "Survive 30 minutes", 400};
    achievementConfigs_[Achievement::FIRST_VICTORY] = {Achievement::FIRST_VICTORY, "Victor", "Win your first match", 500};
    achievementConfigs_[Achievement::DOUBLE_KILL] = {Achievement::DOUBLE_KILL, "Double Kill", "Get 2 kills in a row", 150};
    achievementConfigs_[Achievement::TRIPLE_KILL] = {Achievement::TRIPLE_KILL, "Triple Kill", "Get 3 kills in a row", 300};
}
