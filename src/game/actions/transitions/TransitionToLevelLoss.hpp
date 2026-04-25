#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/setters/SetControlState.hpp"
#include "game/actions/setters/SetPlayerPaddleState.hpp"
#include "game/actions/setters/SetPlayerPaddleVisible.hpp"
#include "game/actions/transitions/TransitionToDefeatedScreen.hpp"
#include "game/actions/ui/PlaySound.hpp"

namespace program {

namespace actions {

class TransitionToLevelLoss : public AbstractAction {
  void act() override {
    State& localState = *this->state;

    localState.controlState = CONTROL_IN_TRANSITION_TO_NEXT_LEVEL;

    insertAction(localState, nullptr, 1000);
    insertAction(localState, new PlaySound("game_over"), 2300);
    insertAction(
        localState, new SetPlayerPaddleState(PLAYER_PADDLE_EXPLODE), 0);
    insertAction(localState, new SetControlState(CONTROL_WAITING), 0);
    insertAction(localState, new PlaySound("ship_expl"), 1500);
    insertAction(localState, new SetPlayerPaddleVisible(false), 0);
    insertAction(localState, new TransitionToDefeatedScreen(), 0);
  }

public:
  TransitionToLevelLoss() {}
};

} // namespace actions

} // namespace program