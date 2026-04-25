#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {
namespace actions {

class Fade : public AbstractAction {
  FadeDirection direction;
  int duration;
  void act() override {
    State& localState = *state;
    localState.uiInfo.fadeDirection = direction;
    timer::start(localState.uiInfo.fadeTimer, duration);
  }

public:
  const std::string musicName;

  Fade(FadeDirection direction, int duration = 1000)
      : direction(direction), duration(duration) {}
};
}; // namespace actions
} // namespace program
