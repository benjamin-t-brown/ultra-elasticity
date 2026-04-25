#pragma once

#include "TransitionBricksIntoLevel.hpp"
#include "TransitionPlayerIntoLevel.hpp"
#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/level/ControlProtectionWalls.hpp"
#include "game/actions/level/SetupBackground.hpp"
#include "game/actions/level/SetupLevel.hpp"
#include "game/actions/setters/SetControlState.hpp"
#include "game/actions/setters/SetPlayerAtStartPosition.hpp"
#include "game/actions/setters/SetPlayerBallCount.hpp"
#include "game/actions/setters/SetPlayerPaddleState.hpp"
#include "game/actions/setters/SetUiState.hpp"
#include "game/actions/spawn/SpawnBallsAtStartOfGame.hpp"
#include "game/actions/spawn/SpawnParticle.hpp"
#include "game/actions/ui/Fade.hpp"
#include "lib/sdl2w/L10n.h"
#include "utils/Gauge.hpp"

namespace program {

namespace actions {

class TransitionToLevelFade : public AbstractAction {
  int level;
  bool useFadeOut;
  bool resetBallCount;
  void act() override {
    State& localState = *this->state;

    localState.controlState = CONTROL_WAITING;
    localState.levelInfo.preventBallLost = false;
    localState.combo = 0;

    if (resetBallCount) {
      // Same level again after a loss: clear juggle progress, turn off
      // protection walls if on, and restore a normal paddle (incl. short
      // paddle timer) before reloading the stage.
      localState.juggleGaugeCompletedTiers = 0;
      localState.juggleGauge.value = 0;
      localState.juggleGaugeMaxBallsSeen = 1;
      gauge::setMax(localState.juggleGauge, 1.);
      timer::start(localState.levelInfo.protectionWallsTimer, 10000.0);
      if (localState.levelInfo.areProtectionWallsEnabled) {
        localState.levelInfo.areProtectionWallsEnabled = false;
      }
      {
        actions::SetPlayerPaddleState toNormal{PLAYER_PADDLE_NORMAL};
        toNormal.setState(&localState);
        toNormal.execute(&localState);
      }
    }

    const int FADE_DURATION = 100;
    const auto playAreaHeight = localState.levelInfo.playAreaHeight;
    const auto playAreaWidth = localState.levelInfo.playAreaWidth;
    auto textParams = sdl2w::RenderTextParams{
        .fontName = "default",
        .fontSize = sdl2w::TextSize::TEXT_SIZE_24,
        .color = {255, 255, 255, 255},
        .centered = true,
    };

    if (useFadeOut) {
      insertAction(
          localState, new Fade(FADE_OUT, FADE_DURATION), FADE_DURATION);
    }
    if (resetBallCount) {
      insertAction(localState,
                   new SetPlayerBallCount(localState.params.numStartingBalls),
                   0);
    }
    insertAction(localState, new SetUiState(UI_HIDDEN), 0);
    insertAction(localState, new SetPlayerAtStartPosition(), 0);
    insertAction(localState, new SetupLevel(level), 100);
    insertAction(localState, new SetupBackground(), 0);
    insertAction(localState, new SetUiState(UI_PRE_GAME), 0);
    insertAction(localState, new Fade(FADE_IN, FADE_DURATION), FADE_DURATION);
    insertAction(localState,
                 new actions::SpawnParticle(TRANSLATE("Level ") +
                                                std::to_string(level + 1),
                                            playAreaWidth / 2.,
                                            playAreaHeight / 2.,
                                            2000,
                                            textParams),
                 0);
    insertAction(localState, new SetUiState(UI_IN_GAME), 0);
    // intended to get called a second time
    insertAction(localState, new SetPlayerAtStartPosition(), 0);
    insertAction(localState, new TransitionBricksIntoLevel(), 0);
    insertAction(localState, new TransitionPlayerIntoLevel(), 0);
    if (useFadeOut) {
      if (!(resetBallCount)) {
        insertAction(localState, new SpawnBallsAtStartOfGame(), 0);
      }
    }
    insertAction(
        //
        localState,
        new SetControlState(CONTROL_IN_GAME),
        0);
  }

public:
  TransitionToLevelFade(int level,
                        bool useFadeOut = false,
                        bool resetBallCount = false)
      : level(level), useFadeOut(useFadeOut), resetBallCount(resetBallCount) {}
};

} // namespace actions

} // namespace program