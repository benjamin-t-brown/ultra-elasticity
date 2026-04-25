#pragma once

#include "game/State.h"
namespace program {

void updateUi(State& state, int dt);
void updateBalls(State& state, int dt);
void updateBricks(State& state, int dt);
void updatePowerups(State& state, int dt);
void updateParticles(State& state, int dt);
void updateGameFlow(State& state, int dt);

// void updateBullet(Bullet& bullet, State& state, int dt);
// void updateBomber(Bomber& bomber, State& state, int dt);
// void updateBomb(Bomb& bomb, State& state, int dt);
// void updateAirplane(Airplane& airplane, State& state, int dt);
// void updateDuoMissile(DuoMissile& missile, State& state, int dt);
// void updateSpawners(State& state, int dt);
// void updateMenu(State& state, int dt);

// inline void playerSetNewWalkTarget(Player& player, State& state) {
//   player.walkX =
//       rand() % state.playAreaWidthTiles * TILE_WIDTH + state.playAreaXOffset;
//   player.walkY = rand() % 4 + state.playAreaBottomYStart + TILE_HEIGHT * 2;
//   timer::start(player.walkTimeoutTimer);
// }
} // namespace program