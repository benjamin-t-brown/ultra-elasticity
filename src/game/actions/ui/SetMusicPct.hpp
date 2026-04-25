#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {
namespace actions {

class SetMusicPct : public AbstractAction {
  int musicPct;

  void act() override {
    State& localState = *state;
    localState.musicPctToSet.push_back(musicPct);
  }

public:
  explicit SetMusicPct(int musicPct) : musicPct(musicPct) {}
};

} // namespace actions
} // namespace program
