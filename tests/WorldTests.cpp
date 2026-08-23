#include "World.h"

#include <cassert>
#include <iostream>

int main() {
    World world(1);
    const PlayerID playerId = world.addPlayer(42, "test-player");
    Player* player = world.getPlayer(playerId);
    assert(player != nullptr);

    player->setVelocity({10.0f, 0.0f, 0.0f});
    const float startX = player->getPosition().x;
    world.update(0.1f);
    const float moved = player->getPosition().x - startX;
    assert(moved > 0.0f && moved <= 1.0f);

    assert(!world.addStructure(999, {0.0f, 0.0f, 0.0f}, StructureType::WALL));
    std::cout << "World tests passed" << std::endl;
    return 0;
}

