#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {

namespace actions {

class SetControlState : public AbstractAction {
  ControlState controlState;
  void act() override {
    State& localState = *this->state;

    localState.controlState = controlState;

    if (controlState == CONTROL_WAITING) {
      localState.playerPaddle.leftPressed = false;
      localState.playerPaddle.rightPressed = false;
    }
  }

public:
  SetControlState(ControlState controlState) : controlState(controlState) {}
};

} // namespace actions

} // namespace program