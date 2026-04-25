#pragma once

#include "TransitionExistingBallsToShip.hpp"
#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/setters/SetControlState.hpp"
#include "game/actions/setters/SetScore.hpp"
#include "game/actions/setters/SetUiState.hpp"
#include "game/actions/ui/Fade.hpp"

namespace program {

namespace actions {

enum SetUiInfoStateVar {
  INNER_SET_UI_INFO_STATE_BASE_SCORE_TEXT_VISIBLE,
  INNER_SET_UI_INFO_STATE_JUGGLE_BONUS_TEXT_VISIBLE,
  INNER_SET_UI_INFO_STATE_SCORE_TEXT_VISIBLE,
  INNER_SET_UI_INFO_STATE_CONTINUE_TEXT_VISIBLE,
};
class InnerSetUiInfoState : public AbstractAction {
  SetUiInfoStateVar var;
  bool value;
  void act() override {
    State& localState = *this->state;
    switch (var) {
    case INNER_SET_UI_INFO_STATE_BASE_SCORE_TEXT_VISIBLE:
      localState.uiInfo.interLevelScreenState.isBaseScoreTextVisible = value;
      break;
    case INNER_SET_UI_INFO_STATE_JUGGLE_BONUS_TEXT_VISIBLE:
      localState.uiInfo.interLevelScreenState.isJuggleBonusTextVisible = value;
      break;
    case INNER_SET_UI_INFO_STATE_SCORE_TEXT_VISIBLE:
      localState.uiInfo.interLevelScreenState.isScoreTextVisible = value;
      break;
    case INNER_SET_UI_INFO_STATE_CONTINUE_TEXT_VISIBLE:
      localState.uiInfo.interLevelScreenState.isContinueTextVisible = value;
      break;
    default:
      break;
    }
  }

public:
  InnerSetUiInfoState(SetUiInfoStateVar var, bool value)
      : var(var), value(value) {}
};

class TransitionToInterLevel : public AbstractAction {

  int level;
  void act() override {
    State& localState = *this->state;

    localState.controlState = CONTROL_IN_TRANSITION_TO_NEXT_LEVEL;
    localState.levelInfo.preventBallLost = false;
    InterLevelScreen::reset(localState.uiInfo.interLevelScreenState);
    localState.uiInfo.nextLevelToTransferTo = level;

    const int FADE_DURATION = 100;

    insertAction(localState, new TransitionExistingBallsToShip(), 0);
    insertAction(localState, new PlaySound("level_complete"), 0);
    insertAction(localState, new SetUiState(UI_INTER_LEVEL), 0);
    insertAction(localState, new Fade(FADE_OUT, FADE_DURATION), FADE_DURATION);
    insertAction(localState, new SetScore(), 0);
    insertAction(localState, nullptr, 500);
    insertAction(localState, new PlaySound("inter_level"), 0);
    insertAction(localState,
                 new InnerSetUiInfoState(
                     INNER_SET_UI_INFO_STATE_BASE_SCORE_TEXT_VISIBLE, true),
                 1000);
    insertAction(localState, new PlaySound("inter_level"), 0);
    insertAction(localState,
                 new InnerSetUiInfoState(
                     INNER_SET_UI_INFO_STATE_JUGGLE_BONUS_TEXT_VISIBLE, true),
                 1000);
    insertAction(localState, new PlaySound("inter_level"), 0);
    insertAction(localState,
                 new InnerSetUiInfoState(
                     INNER_SET_UI_INFO_STATE_SCORE_TEXT_VISIBLE, true),
                 1000);
    insertAction(localState, new PlaySound("inter_level"), 0);
    insertAction(localState,
                 new InnerSetUiInfoState(
                     INNER_SET_UI_INFO_STATE_CONTINUE_TEXT_VISIBLE, true),
                 100);
    insertAction(localState, new SetControlState(CONTROL_INTER_LEVEL), 0);
    // insertAction(localState, new SetupLevel(level), 100);
    // insertAction(localState, new SetupBackground(), 0);
    // insertAction(localState, new SetPlayerAtStartPosition(), 0);
    // insertAction(localState, new SetUiState(UI_PRE_GAME), 0);
    // insertAction(localState, new Fade(FADE_IN, FADE_DURATION),
    // FADE_DURATION); insertAction(localState, new SetUiState(UI_IN_GAME), 0);
    // insertAction(localState, new TransitionBricksIntoLevel(), 0);
    // insertAction(localState, new TransitionPlayerIntoLevel(), 0);
    // // insertAction(localState, new TransitionBallIntoLevel(), 0);
    // insertAction(
    //     //
    //     localState,
    //     new SetControlState(CONTROL_IN_GAME),
    //     0);
  }

public:
  TransitionToInterLevel(int level) : level(level) {}
};

} // namespace actions

} // namespace program