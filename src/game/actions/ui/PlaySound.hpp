#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {
namespace actions {

class PlaySound : public AbstractAction {
  void act() override {
    State& localState = *state;
    LOG(INFO) << "Playing sound: " << soundName << LOG_ENDL;
    localState.soundsToPlay.push_back(soundName);
  }

public:
  const std::string soundName;

  PlaySound(const std::string& soundName) : soundName(soundName) {}
};
}; // namespace actions
} // namespace program