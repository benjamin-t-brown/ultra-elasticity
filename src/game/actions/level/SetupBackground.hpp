#pragma once

#include "game/Data.h"
#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {

namespace actions {

class SetupBackground : public AbstractAction {
  void act() override {
    State& localState = *this->state;
    loadBackground(localState, localState.terrainIndex);
  }
};

} // namespace actions

} // namespace program