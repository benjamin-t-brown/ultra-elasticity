#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/spawn/SpawnBall.hpp"
#include "game/actions/transitions/TransitionExistingBallsToShip.hpp"
#include "game/actions/ui/PlaySound.hpp"

namespace program {

namespace actions {

class SpawnBallsAtStartOfGame : public AbstractAction {
  BallType ballType;
  void act() override {
    State& localState = *this->state;
    const int numBalls = localState.params.numStartingBalls;
    for (int i = 0; i < numBalls; i++) {
      insAct(new PlaySound("inter_level"), 0);
      insAct(new actions::SpawnBall(
                 //
                 37.,
                 localState.playerPaddle.physics.y - 30,
                 0.37,
                 0.,
                 0.27,
                 ballType,
                 false),
             150);
    }
    insAct(new TransitionExistingBallsToShip(), 0);
  }

public:
  SpawnBallsAtStartOfGame(BallType ballType = BALL_TYPE_NORMAL)
      : ballType(ballType) {}
};

} // namespace actions

} // namespace program
