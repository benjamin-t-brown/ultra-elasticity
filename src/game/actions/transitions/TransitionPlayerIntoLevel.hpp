#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/setters/SetPlayerPaddleState.hpp"
#include "game/actions/setters/SetPlayerPaddleVisible.hpp"
#include "game/actions/ui/PlaySound.hpp"

namespace program {

namespace actions {

class TransitionPlayerIntoLevel : public AbstractAction {
  void act() override {
    State& localState = *this->state;
    std::vector<std::string> animNames = {
        "player2_intro",
        "paddle2_intro",
        "paddle2_sparkle",
    };

    for (auto& animName : animNames) {
      auto& anim = localState.playerPaddle.getAnim(*localState.store, animName);
      anim.start();
    }

    addParallelAction(localState, new PlaySound("ship_intro"), 0);
    addParallelAction(localState, new SetPlayerPaddleVisible(true), 100);
    insertAction(
        //
        localState,
        new SetPlayerPaddleState(PLAYER_PADDLE_SPAWNING),
        1000);
    insertAction(
        //
        localState,
        new SetPlayerPaddleState(PLAYER_PADDLE_NORMAL),
        0);
  }

public:
  TransitionPlayerIntoLevel() {}
};

} // namespace actions

} // namespace program