#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/setters/SetBrickTransform.hpp"

namespace program {

namespace actions {

class TransitionBricksIntoLevel : public AbstractAction {

  void act() override {
    State& localState = *this->state;
    int brickDuration = 500;
    // set bricks to be invisible
    for (auto& brick : localState.bricks) {
      insertAction(
          localState,
          new actions::SetBrickTransform(brick.get(), -70, -70, brickDuration),
          33);
    }
    insertAction(localState, nullptr, brickDuration - 33);
  }
};

} // namespace actions

} // namespace program
