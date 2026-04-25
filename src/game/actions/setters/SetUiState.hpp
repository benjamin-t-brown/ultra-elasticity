#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {

namespace actions {

class SetUiState : public AbstractAction {
  UiState uiState;
  void act() override {
    State& localState = *this->state;

    localState.uiState = uiState;
  }

public:
  SetUiState(UiState uiState) : uiState(uiState) {}
};

} // namespace actions

} // namespace program