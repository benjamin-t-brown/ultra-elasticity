#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/ui/PlaySound.hpp"

namespace program {

namespace actions {

class SetBrickTransform : public AbstractAction {
  Brick* brick;
  int startX;
  int startY;
  int duration;
  void act() override {
    auto& localState = *state;
    if (!getBrickByPtr(localState, brick)) {
      return;
    }
    brick->isVisible = true;
    brick->transform.setStartPos(startX, startY);
    brick->transform.setEndPos(brick->physics.x, brick->physics.y);
    brick->transform.start(duration);
    addParallelAction(localState, new actions::PlaySound("brick_transition"), 0);
    // addParallelAction(localState,
    //                   std::unique_ptr<actions::AbstractAction>(
    //                       new actions::PlaySound("brick_transition_end")),
    //                   duration - 33);
  }

public:
  SetBrickTransform(Brick* brick,
                    int startX = -70,
                    int startY = -70,
                    int duration = 500)
      : brick(brick), startX(startX), startY(startY), duration(duration) {}
};

} // namespace actions

} // namespace program