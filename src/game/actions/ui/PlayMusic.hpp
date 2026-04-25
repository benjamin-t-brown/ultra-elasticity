#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {
namespace actions {

class PlayMusic : public AbstractAction {
  void act() override {
    State& localState = *state;
    localState.musicToPlay.push_back(musicName);
  }

public:
  const std::string musicName;

  PlayMusic(const std::string& musicName) : musicName(musicName) {}
};
}; // namespace actions
} // namespace program
