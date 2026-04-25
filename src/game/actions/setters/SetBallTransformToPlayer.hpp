#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/ui/PlaySound.hpp"

namespace program {

namespace actions {

class SetBallTransformToPlayer : public AbstractAction {
  Ball* ball;
  void act() override {
    auto& localState = *state;
    if (!getBallByPtr(localState, ball)) {
      return;
    }
    ball->isTransitioningToPlayer = true;
    ball->ballTransitionPrevX = ball->physics.x;
    ball->ballTransitionPrevY = ball->physics.y;
    ball->physics.friction = 0.;
    timer::start(ball->transitionToPlayerTimer);
  }

public:
  SetBallTransformToPlayer(Ball* ball) : ball(ball) {}
};

} // namespace actions

} // namespace program