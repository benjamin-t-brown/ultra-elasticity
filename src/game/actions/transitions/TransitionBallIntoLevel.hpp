#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/spawn/SpawnBall.hpp"
#include "game/actions/ui/PlaySound.hpp"

namespace program {

namespace actions {

class TransitionBallIntoLevel : public AbstractAction {
  BallType ballType;
  void act() override {
    State& localState = *this->state;
    localState.playerPaddle.numBalls--;
    localState.uiInfo.showLaunchText = true;
    localState.uiInfo.launchBallText.isVisible = true;
    localState.uiInfo.showTextIndicatingBallIsReadyToLaunch = false;
    timer::start(localState.uiInfo.launchBallText.timer);
    insertAction(localState,
                 new actions::SpawnBall(
                     //
                     localState.playerPaddle.physics.x,
                     480. - (32. + 16. + 24 + 16),
                     0.,
                     0.,
                     0.27,
                     ballType,
                     true),
                 0);
    insertAction(localState, new PlaySound("ball_spawn"), 0);
  }

public:
  TransitionBallIntoLevel(BallType ballType = BALL_TYPE_NORMAL)
      : ballType(ballType) {}
};

} // namespace actions

} // namespace program
