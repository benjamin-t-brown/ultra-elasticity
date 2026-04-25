#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {

namespace actions {

class SetPlayerPaddleVisible : public AbstractAction {
  bool isVisible;
  void act() override {
    State& localState = *this->state;

    localState.playerPaddle.isVisible = isVisible;
  }

public:
  SetPlayerPaddleVisible(bool isVisible) : isVisible(isVisible) {}
};

} // namespace actions

} // namespace program