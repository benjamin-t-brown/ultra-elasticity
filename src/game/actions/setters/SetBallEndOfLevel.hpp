#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/ui/PlaySound.hpp"

namespace program {

namespace actions {

class SetBallEndOfLevel : public AbstractAction {
  Ball* ball;
  void act() override {
    auto& localState = *state;
    if (!getBallByPtr(localState, ball)) {
      return;
    }
    ball->isHighlighted = true;
    ball->physics.friction = 0.006;
    ball->numPreviousStates = 7;
    ball->prevStateTimer.duration = 40;
    auto& anim = ball->getAnim(*localState.store, "ball_highlighted");
    anim.start();
    addParallelAction(localState, new PlaySound("ball_highlight"), 100);
  }

public:
  SetBallEndOfLevel(Ball* ball) : ball(ball) {}
};

} // namespace actions

} // namespace program