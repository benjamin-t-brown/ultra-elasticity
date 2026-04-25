#pragma once

#include "game/Data.h"
#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/spawn/SpawnBall.hpp"

namespace program {

namespace actions {

class SetupLevel : public AbstractAction {
  int level;
  void act() override {
    State& localState = *this->state;

    localState.balls.erase(localState.balls.begin(), localState.balls.end());
    localState.powerups.erase(localState.powerups.begin(),
                              localState.powerups.end());
    localState.bricks.erase(localState.bricks.begin(), localState.bricks.end());
    localState.particles.clear();

    localState.levelInfo.levelIndex = level;

    loadLevel(localState, level);

    for (auto& brick : localState.bricks) {
      brick->isVisible = false;
    }
  }

public:
  SetupLevel(int level) : level(level) {}
};

} // namespace actions

} // namespace program