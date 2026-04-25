#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {

namespace actions {

class SetPlayerBallCount : public AbstractAction {
  int ballCount;
  void act() override {
    State& localState = *this->state;

    localState.playerPaddle.numBalls = ballCount;
  }

public:
  SetPlayerBallCount(int ballCount) : ballCount(ballCount) {}
};

} // namespace actions

} // namespace program