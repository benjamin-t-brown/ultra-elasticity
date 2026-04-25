#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {

namespace actions {

class TransitionToDefeatedScreen : public AbstractAction {
  void act() override {
    State& localState = *this->state;
    localState.controlState = CONTROL_DEFEATED;
    localState.uiState = UI_DEFEATED;
    timer::start(localState.uiInfo.defeatedAutoMenuTimer, 5000.0);
    localState.uiInfo.defeatedRestartPrompt.reset();
  }

public:
  TransitionToDefeatedScreen() = default;
};

} // namespace actions

} // namespace program
