#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/ui/PlaySound.hpp"

namespace program {

namespace actions {

class ControlProtectionWalls : public AbstractAction {
  bool areProtectionWallsEnabled;
  void act() override {
    State& localState = *this->state;
    const int WALL_SIZE = 6;
    localState.levelInfo.areProtectionWallsEnabled = areProtectionWallsEnabled;
    if (areProtectionWallsEnabled) {
      timer::start(localState.levelInfo.protectionWallsTimer, 10000);
      enqueueAction(localState, new actions::PlaySound("brick_armor_hit"), 0);
      localState.levelInfo.playAreaXOffset = WALL_SIZE;
      localState.levelInfo.playAreaYOffset = WALL_SIZE;
      localState.levelInfo.playAreaWidth =
          localState.levelInfo.defaultPlayAreaWidth - WALL_SIZE * 2;
      localState.levelInfo.playAreaHeight =
          localState.levelInfo.defaultPlayAreaHeight - WALL_SIZE * 2;
    } else {
      enqueueAction(localState, new actions::PlaySound("brick_armor_end"), 0);
      localState.levelInfo.playAreaXOffset =
          localState.levelInfo.defaultPlayAreaXOffset;
      localState.levelInfo.playAreaYOffset =
          localState.levelInfo.defaultPlayAreaYOffset;
      localState.levelInfo.playAreaWidth =
          localState.levelInfo.defaultPlayAreaWidth;
      localState.levelInfo.playAreaHeight =
          localState.levelInfo.defaultPlayAreaHeight;
    }
  }

public:
  ControlProtectionWalls(bool areProtectionWallsEnabled)
      : areProtectionWallsEnabled(areProtectionWallsEnabled) {}
};

} // namespace actions

} // namespace program