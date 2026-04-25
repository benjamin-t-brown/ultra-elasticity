#pragma once

#include "TransitionBricksIntoLevel.hpp"
#include "TransitionExistingBallsToShip.hpp"
#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/level/SetupLevel.hpp"
#include "game/actions/setters/SetControlState.hpp"
#include "game/actions/setters/SetUiState.hpp"
#include "game/actions/spawn/SpawnParticle.hpp"
#include "lib/sdl2w/L10n.h"

namespace program {

namespace actions {

class TransitionToLevelLive : public AbstractAction {
  int level;
  void act() override {
    State& localState = *this->state;

    LOG(INFO) << "Transitioning to level live: " << level << LOG_ENDL;

    localState.levelInfo.preventBallLost = false;
    localState.controlState = CONTROL_IN_TRANSITION_TO_NEXT_LEVEL;
    localState.combo = 0;

    const auto playAreaHeight = localState.levelInfo.playAreaHeight;
    const auto playAreaWidth = localState.levelInfo.playAreaWidth;
    auto textParams = sdl2w::RenderTextParams{
        .fontName = "default",
        .fontSize = sdl2w::TextSize::TEXT_SIZE_24,
        .color = {255, 255, 255, 255},
        .centered = true,
    };

    insertAction(localState, new TransitionExistingBallsToShip(), 0);
    insertAction(localState, new SetupLevel(level), 0);
    insertAction(localState, new SetUiState(UI_IN_GAME), 0);
    insertAction(localState, new TransitionBricksIntoLevel(), 0);
    insertAction(localState,
                 new actions::SpawnParticle(TRANSLATE("Level ") +
                                                std::to_string(level + 1),
                                            playAreaWidth / 2.,
                                            playAreaHeight / 2.,
                                            4000,
                                            textParams),
                 0);
    // insertAction(localState, new TransitionBallIntoLevel(), 0);
    insertAction(localState, new SetControlState(CONTROL_IN_GAME), 0);
  }

public:
  TransitionToLevelLive(int level) : level(level) {}
};

} // namespace actions

} // namespace program