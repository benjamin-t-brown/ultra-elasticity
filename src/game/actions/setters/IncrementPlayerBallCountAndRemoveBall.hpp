#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {

namespace actions {

class IncrementPlayerBallCountAndRemoveBall : public AbstractAction {
  Ball* ball;
  void act() override {
    auto& localState = *state;
    if (!getBallByPtr(localState, ball)) {
      return;
    }
    localState.playerPaddle.numBalls++;
    ball->shouldRemove = true;
  }

public:
  IncrementPlayerBallCountAndRemoveBall(Ball* ball) : ball(ball) {}
};

} // namespace actions

} // namespace program