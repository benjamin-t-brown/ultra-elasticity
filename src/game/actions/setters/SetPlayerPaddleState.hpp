#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/ui/PlaySound.hpp"

namespace program {

namespace actions {

class SetPlayerPaddleState : public AbstractAction {
  PlayerPaddleState playerPaddleState;
  void act() override {
    State& localState = *this->state;
    const PlayerPaddleState prevState = localState.playerPaddle.paddleState;

    localState.playerPaddle.paddleState = playerPaddleState;
    if (playerPaddleState != PLAYER_PADDLE_SHORT) {
      localState.playerPaddle.shortPaddleTimer.t =
          localState.playerPaddle.shortPaddleTimer.duration;
    }
    applyPaddleCollisionProfile(localState.playerPaddle, playerPaddleState);
    if (prevState == PLAYER_PADDLE_SHORT &&
        playerPaddleState == PLAYER_PADDLE_NORMAL) {
      addParallelAction(localState, new PlaySound("restore_paddle"), 0);
    }
    if (playerPaddleState == PLAYER_PADDLE_EXPLODE) {
      auto& anim =
          localState.playerPaddle.getAnim(*localState.store, "player2_explode");
      anim.start();
    }
  }

public:
  SetPlayerPaddleState(PlayerPaddleState playerPaddleState)
      : playerPaddleState(playerPaddleState) {}
};

} // namespace actions

} // namespace program