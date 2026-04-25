#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/spawn/SpawnParticle.hpp"
#include "game/actions/ui/PlaySound.hpp"

namespace program {
struct Ball;
} // namespace program

namespace program::actions {

class DoCollisionBallLost : public AbstractAction {
  Ball* ball;
  void act() override {
    State& localState = *this->state;
    auto ballOpt = getBallByPtr(localState, ball);
    if (!ballOpt) {
      return;
    };
    Ball& b = **ballOpt;
    b.shouldRemove = true;
    addParallelAction(
        localState,
        new actions::SpawnParticle("ball_out",
                                   b.physics.x,
                                   localState.levelInfo.playAreaHeight - 64. / 2.,
                                   1000),
        0);
    addParallelAction(localState, new PlaySound("ball_expl"), 0);
    addParallelAction(localState, new PlaySound("ball_lost"), 500);
  }

public:
  explicit DoCollisionBallLost(Ball* ball) : ball(ball) {}
};

} // namespace program::actions
