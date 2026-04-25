#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/ui/SetMusicPct.hpp"
#include <algorithm>

namespace program {
namespace actions {

class StopMusicNow : public AbstractAction {
  void act() override {
    State& localState = *state;
    localState.stopMusic = true;
  }
};

class StopMusic : public AbstractAction {
  int initialMusicPct;
  int durationMs;

  void act() override {
    if (initialMusicPct <= 0 || durationMs <= 0) {
      insAct(new StopMusicNow(), 0);
      return;
    }

    constexpr int kNumFadeSteps = 10;
    const int stepDurationMs = std::max(1, durationMs / kNumFadeSteps);
    for (int i = 1; i <= kNumFadeSteps; i++) {
      const int nextPct = initialMusicPct * (kNumFadeSteps - i) / kNumFadeSteps;
      insAct(new SetMusicPct(nextPct), stepDurationMs);
    }
    insAct(new StopMusicNow(), 0);
    // Restore configured volume for future tracks after current music halts.
    insAct(new SetMusicPct(initialMusicPct), 0);
  }

public:
  explicit StopMusic(int initialMusicPct, int durationMs = 1000)
      : initialMusicPct(initialMusicPct), durationMs(durationMs) {}
};

} // namespace actions
} // namespace program
