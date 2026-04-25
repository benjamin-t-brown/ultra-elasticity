#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/setters/IncrementPlayerBallCountAndRemoveBall.hpp"
#include "game/actions/setters/SetBallEndOfLevel.hpp"
#include "game/actions/setters/SetBallTransformToPlayer.hpp"
#include "game/actions/setters/SetPlayerPaddleState.hpp"
#include "game/actions/ui/PlaySound.hpp"

namespace program {

namespace actions {

class TransitionExistingBallsToShip : public AbstractAction {
  void act() override {
    State& localState = *this->state;

    auto& anim =
        localState.playerPaddle.getAnim(*localState.store, "player2_flash");
    anim.start();

    int ballDuration = 0;
    for (auto& ball : localState.balls) {
      insertAction(localState, new SetBallEndOfLevel(ball.get()), 0);
      addParallelAction(localState, new SetBallTransformToPlayer(ball.get()), 1000);
      addParallelAction(
          localState,
          new PlaySound("ball_transfer"),
          1000);
      addParallelAction(
          localState,
          new IncrementPlayerBallCountAndRemoveBall(ball.get()),
          1000 + ball->transitionToPlayerTimer.duration);
      ballDuration = 1000 + ball->transitionToPlayerTimer.duration;
    }
    addParallelAction(localState,
                      new SetPlayerPaddleState(PLAYER_PADDLE_FLASH),
                      ballDuration + 33);
    addParallelAction(
        localState,
        new PlaySound("player_absorb_ball"),
        ballDuration + 100);
    addParallelAction(localState,
                      new SetPlayerPaddleState(PLAYER_PADDLE_NORMAL),
                      ballDuration + 700);
    insertAction(localState, nullptr, ballDuration + 700 + 33);
  }

public:
  TransitionExistingBallsToShip() {}
};

} // namespace actions

} // namespace program