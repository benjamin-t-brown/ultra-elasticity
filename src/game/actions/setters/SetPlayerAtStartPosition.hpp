#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {

namespace actions {

class SetPlayerAtStartPosition : public AbstractAction {
  void act() override {
    State& localState = *this->state;
    int w = localState.levelInfo.playAreaWidth + localState.levelInfo.playAreaXOffset;
    int h = localState.levelInfo.playAreaHeight + localState.levelInfo.playAreaYOffset;
    localState.playerPaddle.isVisible = false;
    localState.playerPaddle.physics.x = w / 2.;
    localState.playerPaddle.physics.y = h - (32. + 16.);
    localState.playerPaddle.paddleState = PLAYER_PADDLE_SPAWNING;
  }

public:
  SetPlayerAtStartPosition() {}
};

} // namespace actions

} // namespace program