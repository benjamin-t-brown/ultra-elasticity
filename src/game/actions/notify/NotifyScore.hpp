#pragma once

#include "game/actions/AbstractAction.h"
#include "lib/sdl2w/EmscriptenHelpers.h"

namespace program {

namespace actions {

class NotifyGameCompleted : public AbstractAction {
  int score;
  int maxCombo;
  int time;
  void act() override {
    std::stringstream ss;
    ss << score << " " << maxCombo << " " << time;
    emshelpers::notifyGameCompleted(ss.str());
  }

public:
  NotifyGameCompleted(int score, int maxCombo, int time)
      : score(score), maxCombo(maxCombo), time(time) {}
};

} // namespace actions

} // namespace program