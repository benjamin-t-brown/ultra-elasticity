#pragma once

#include "game/Data.h"
#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/spawn/SpawnBall.hpp"
#include "game/actions/ui/Fade.hpp"
#include "game/actions/ui/PlayMusic.hpp"

namespace program {

namespace actions {

class TransitionToMenu : public AbstractAction {
  bool useFade;
  void act() override {
    State& localState = *this->state;

    localState.controlState = CONTROL_MENU;
    localState.uiState = UI_MENU;
    localState.background.terrainIndex = 0;
    localState.background.offset = 0.0;
    localState.playerPaddle.isVisible = false;
    localState.levelInfo.preventBallLost = true;

    localState.balls.erase(localState.balls.begin(), localState.balls.end());
    localState.powerups.erase(localState.powerups.begin(),
                              localState.powerups.end());
    localState.bricks.erase(localState.bricks.begin(), localState.bricks.end());

    loadBackground(localState, 0);
    if (useFade) {
      insertAction(localState, new actions::Fade(FADE_IN, 100), 100);
    }
    insertAction(
        //
        localState,
        new actions::SpawnBall(640. / 2.,
                               480. - 64. - 16.,
                               0.15,
                               0.19,
                               1.5,
                               BALL_TYPE_NORMAL,
                               false),
        0);

    if (localState.params.playMenuMusic) {
      enqueueAction(localState, new actions::PlayMusic("menu"), 0);
    }
  }

public:
  TransitionToMenu(bool useFade = false) : useFade(useFade) {}
};

} // namespace actions

} // namespace program