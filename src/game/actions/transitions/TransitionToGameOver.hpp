#pragma once

#include "TransitionExistingBallsToShip.hpp"
#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/setters/SetControlState.hpp"
#include "game/actions/setters/SetScore.hpp"
#include "game/actions/setters/SetUiState.hpp"
#include "game/actions/ui/Fade.hpp"
#include "lib/hiscore/hiscore.h"
#include <algorithm>

namespace program {

namespace actions {

namespace {
constexpr int kEndGameScorePenaltyPerRetry = 5000;
}

class TransitionToGameOver : public AbstractAction {
  enum SetVar {
    SET_VAR_SHOW_SCORE,
    SET_VAR_SHOW_MAX_COMBO,
    SET_VAR_SHOW_BASE_SCORE,
    SET_VAR_SHOW_JUGGLE_BONUS,
    SET_VAR_SHOW_TIME,
    SET_VAR_SHOW_RETRIES,
    SET_VAR_SHOW_CONTINUE_TEXT,
    SET_VAR_SHOW_HISCORE_COMBO,
    SET_VAR_SHOW_HISCORE_TIME,
    SET_VAR_SHOW_HISCORE_SCORE,
  };
  class SetBoolVarAction : public AbstractAction {
    SetVar var;
    bool value;
    void act() override {
      State& localState = *this->state;
      switch (var) {
      case SET_VAR_SHOW_MAX_COMBO:
        localState.uiInfo.endGameScreenState.isComboTextVisible = value;
        break;
      case SET_VAR_SHOW_BASE_SCORE:
        localState.uiInfo.endGameScreenState.isBaseScoreTextVisible = value;
        break;
      case SET_VAR_SHOW_JUGGLE_BONUS:
        localState.uiInfo.endGameScreenState.isJuggleBonusTextVisible = value;
        break;
      case SET_VAR_SHOW_TIME:
        localState.uiInfo.endGameScreenState.isTimeTextVisible = value;
        break;
      case SET_VAR_SHOW_RETRIES:
        localState.uiInfo.endGameScreenState.isRetriesTextVisible = value;
        break;
      case SET_VAR_SHOW_CONTINUE_TEXT:
        localState.uiInfo.endGameScreenState.isPressButtonTextVisible = value;
        break;
      case SET_VAR_SHOW_SCORE:
        localState.uiInfo.endGameScreenState.isScoreTextVisible = value;
        break;
      case SET_VAR_SHOW_HISCORE_COMBO:
        localState.uiInfo.endGameScreenState.isNewBestComboTextVisible = value;
        break;
      case SET_VAR_SHOW_HISCORE_TIME:
        localState.uiInfo.endGameScreenState.isNewBestTimeTextVisible = value;
        break;
      case SET_VAR_SHOW_HISCORE_SCORE:
        localState.uiInfo.endGameScreenState.isNewBestScoreTextVisible = value;
        break;
      default:
        break;
      }
    }

  public:
    SetBoolVarAction(SetVar var, bool value) : var(var), value(value) {}
  };

  class TransitionPlayerOffScreen : public AbstractAction {
    void act() override {
      State& localState = *this->state;
      localState.playerPaddle.transform.setStartPos(
          localState.playerPaddle.physics.x, localState.playerPaddle.physics.y);
      localState.playerPaddle.transform.setEndPos(
          localState.playerPaddle.physics.x, -42);
      localState.playerPaddle.transform.start(3000);
    }
  };

  class SetLastScore : public AbstractAction {
    void act() override {
      State& localState = *this->state;
      const int retryPenalty = localState.numContinuesUsed * kEndGameScorePenaltyPerRetry;
      const int withPenalty =
          std::max(0, localState.score - retryPenalty);
      localState.lastScore = withPenalty;
      localState.score = withPenalty;
      localState.lastMaxCombo = localState.maxCombo;
      localState.lastTime = localState.stopwatchMs / 1000;
    }
  };

  hiscore::HiscoreRow getBestHiscoreRow() {
    auto hiscores = hiscore::getHighScores();
    int previousBestScore = 0;
    int previousBestMaxCombo = 0;
    int previousBestTimeSeconds = 0;
    for (const auto& hiscore : hiscores) {
      if (hiscore.score > previousBestScore) {
        previousBestScore = hiscore.score;
      }
      if (hiscore.maxCombo > previousBestMaxCombo) {
        previousBestMaxCombo = hiscore.maxCombo;
      }
      if (hiscore.time > 0 && (previousBestTimeSeconds == 0 ||
                               hiscore.time < previousBestTimeSeconds)) {
        previousBestTimeSeconds = hiscore.time;
      }
    }
    return hiscore::HiscoreRow{"PLAYER",
                               previousBestScore,
                               previousBestMaxCombo,
                               previousBestTimeSeconds};
  }

  void act() override {
    State& localState = *this->state;

    localState.combo = 0;
    localState.controlState = CONTROL_IN_TRANSITION_TO_NEXT_LEVEL;
    localState.levelInfo.preventBallLost = false;
    EndGameScreen::reset(localState.uiInfo.endGameScreenState);

    const auto bestScores = getBestHiscoreRow();
    const int currentTime = localState.stopwatchMs / 1000;
    const int currentMaxCombo = localState.maxCombo;
    const int rollUpBeforeRetries =
        localState.stageStartScore + localState.stageScore +
        localState.stageScore * localState.juggleGaugeCompletedTiers +
        currentMaxCombo * 1000;
    const int currentScore = std::max(
        0, rollUpBeforeRetries -
               localState.numContinuesUsed * kEndGameScorePenaltyPerRetry);

    bool didGetNewBestScore = currentScore > bestScores.score;
    bool didGetNewBestMaxCombo = currentMaxCombo > bestScores.maxCombo;
    bool didGetNewBestTime = currentTime < bestScores.time;

    const hiscore::HiscoreRow nextHighScore = {
        "PLAYER",
        didGetNewBestScore ? currentScore : bestScores.score,
        didGetNewBestMaxCombo ? currentMaxCombo : bestScores.maxCombo,
        didGetNewBestTime ? currentTime : bestScores.time};

    hiscore::saveHighScores(std::vector<hiscore::HiscoreRow>{nextHighScore});

    insAct(new TransitionExistingBallsToShip(), 0);
    insAct(new TransitionPlayerOffScreen(), 0);
    insAct(new PlaySound("game_victory"), 0);
    insAct(new SetUiState(UI_GAME_OVER), 0);
    insAct(new Fade(FADE_OUT, 2800), 2800);
    insAct(new SetScore(true), 0);
    insAct(new SetLastScore(), 0);
    insAct(nullptr, 700);
    insAct(new PlaySound("inter_level"), 0);
    insAct(new SetBoolVarAction(SET_VAR_SHOW_BASE_SCORE, true), 600);
    insAct(new PlaySound("inter_level"), 0);
    insAct(new SetBoolVarAction(SET_VAR_SHOW_JUGGLE_BONUS, true), 600);
    insAct(new PlaySound("inter_level"), 0);
    insAct(new SetBoolVarAction(SET_VAR_SHOW_MAX_COMBO, true), 600);
    if (didGetNewBestMaxCombo) {
      insAct(new SetBoolVarAction(SET_VAR_SHOW_HISCORE_COMBO, true), 0);
      insAct(new PlaySound("hiscore_combo"), 0);
      insAct(nullptr, 600);
    }

    insAct(new PlaySound("inter_level"), 0);
    insAct(new SetBoolVarAction(SET_VAR_SHOW_TIME, true), 600);
    if (didGetNewBestTime) {
      insAct(new SetBoolVarAction(SET_VAR_SHOW_HISCORE_TIME, true), 0);
      insAct(new PlaySound("hiscore_time"), 0);
      insAct(nullptr, 600);
    }

    if (localState.numContinuesUsed > 0) {
      insAct(new PlaySound("powerup_bad"), 0);
      insAct(new SetBoolVarAction(SET_VAR_SHOW_RETRIES, true), 600);
    }

    insAct(new PlaySound("inter_level"), 0);
    insAct(new SetBoolVarAction(SET_VAR_SHOW_SCORE, true), 600);
    if (didGetNewBestScore) {
      insAct(new SetBoolVarAction(SET_VAR_SHOW_HISCORE_SCORE, true), 0);
      insAct(new PlaySound("hiscore_score"), 0);
      insAct(nullptr, 600);
    }

    insAct(new SetBoolVarAction(SET_VAR_SHOW_CONTINUE_TEXT, true), 100);
    insAct(new SetControlState(CONTROL_GAME_OVER), 0);
  }

public:
  TransitionToGameOver() {}
};

} // namespace actions

} // namespace program