#include "GameManager.h"
#include "State.h"
#include "actions/notify/NotifyScore.hpp"
#include "actions/setters/ResetStageScore.hpp"
#include "actions/transitions/TransitionBallIntoLevel.hpp"
#include "actions/transitions/TransitionToLevelFade.hpp"
#include "actions/transitions/TransitionToMenu.hpp"
#include "actions/ui/StopMusic.hpp"
#include "client/Keys.hpp"
#include "client/Render.h"
#include "client/UiRenderer.h"
#include "lib/hiscore/hiscore.h"
#include "lib/sdl2w/EmscriptenHelpers.h"
#include "lib/sdl2w/Window.h"
#include "updaters/CheckCollisions.h"
#include "updaters/UpdateBackground.h"
#include "updaters/UpdateEntities.h"
#include "updaters/UpdatePlayer.h"
#include <unordered_set>

namespace program {

enum ExternalEvent {
  EXTERNAL_EVENT_OVERRIDE_HISCORE = 0,
};

GameManager::GameManager(sdl2w::Window& windowA, GameParams gameParams)
    : window(windowA), r(windowA) {
  state.params = gameParams;
}

GameManager::~GameManager() {}

void GameManager::load() {
  LOG(INFO) << "Loading level data from file..." << LOG_ENDL;
  loadLevelDataFromFile(state.levels);
  LOG(INFO) << "Level data loaded." << LOG_ENDL;
  state.store = &window.getStore();
  r.setup(state);
}

void GameManager::start() {
  enqueueAction(state, new actions::TransitionToMenu(), 0);
  // load high scores
  auto hiscores = hiscore::getHighScores();
  int bestScore = 0;
  int bestMaxCombo = 0;
  int bestTime = 0;
  for (auto& hiscore : hiscores) {
    if (hiscore.score > bestScore) {
      bestScore = hiscore.score;
    }
    if (hiscore.maxCombo > bestMaxCombo) {
      bestMaxCombo = hiscore.maxCombo;
    }
    if (hiscore.time < bestTime || bestTime == 0) {
      bestTime = hiscore.time;
    }
  }
  state.bestScore = bestScore;
  state.bestMaxCombo = bestMaxCombo;
  state.bestTime = bestTime;
  emshelpers::notifyGameReady();
}

void GameManager::handleKeyPress(const std::string& key) {
  // LOG(INFO) << "Key pressed: " << key << LOG_ENDL;
  switch (state.controlState) {
  case CONTROL_MENU:
    if (isConfirmKey(key)) {
      state.controlState = CONTROL_WAITING;
      state.playerPaddle.numBalls = 0;
      state.score = 0;
      state.stageScore = 0;
      state.combo = 0;
      state.maxCombo = 0;
      state.interLevelJuggleBonus = 0;
      state.interLevelJuggleTier = 0;
      state.totalJuggleBonus = 0;
      state.juggleGaugeMaxBallsSeen = 1;
      state.juggleGaugeCompletedTiers = 0;
      state.juggleGauge.value = 0;
      state.stopwatchMs = 0;
      state.numContinuesUsed = 0;
      addParallelAction(
          state, new actions::StopMusic(window.getMusicPct(), 500), 0);
      enqueueAction(state, new actions::ResetStageScore(), 0);
      enqueueAction(state, new actions::PlaySound("game_start"), 0);
      enqueueAction(
          state,
          new actions::TransitionToLevelFade(state.params.startingLevel, true),
          0);
      emshelpers::notifyGameStarted();
    }
    break;
  case CONTROL_IN_GAME:
    if (isConfirmKey(key)) {
      if (state.playerPaddle.numBalls > 0) {
        enqueueAction(
            state, new actions::TransitionBallIntoLevel(BALL_TYPE_NORMAL), 0);
      }
    }
    break;
  case CONTROL_INTER_LEVEL:
    if (isConfirmKey(key)) {
      enqueueAction(state, new actions::ResetStageScore(), 0);
      enqueueAction(state,
                    new actions::TransitionToLevelFade(
                        state.uiInfo.nextLevelToTransferTo, false),
                    0);
    }
    break;
  case CONTROL_DEFEATED:
    if (isConfirmKey(key)) {
      state.controlState = CONTROL_WAITING;
      state.numContinuesUsed++;
      enqueueAction(state,
                    new actions::TransitionToLevelFade(
                        state.levelInfo.levelIndex, true, true),
                    0);
    }
    break;
  case CONTROL_GAME_OVER:
    if (isConfirmKey(key)) {
      enqueueAction(state,
                    new actions::NotifyGameCompleted(
                        state.lastScore, state.lastMaxCombo, state.lastTime),
                    0);
      enqueueAction(state, new actions::TransitionToMenu(true), 0);
    }
    break;
  default:
    break;
  }
}

void GameManager::handleKeyRelease(const std::string& key) {
  // LOG(INFO) << "Key released: " << key << LOG_ENDL;
  switch (state.controlState) {
  case CONTROL_IN_GAME:
    if (isConfirmKey(key)) {
      for (auto& ball : state.balls) {
        if (ball->isStickToPlayer) {
          ball->isStickToPlayer = false;
          ball->physics.vy = ball->speed;
          state.uiInfo.showLaunchText = false;
          enqueueAction(state, new actions::PlaySound("ball_launch"), 0);
        }
      }
    }
    break;
  case CONTROL_GAME_OVER:
    break;
  default:
    break;
  }
}

void GameManager::update(int dt) {
  const int fixedDt = state.params.fixedDt;

  window.processExternalEvents([this](int event, std::string payload) {
    switch (event) {
    case EXTERNAL_EVENT_OVERRIDE_HISCORE:
      auto scores = hiscore::parseHiscoreText(payload);
      if (scores.size() > 0) {
        auto firstRow = scores[0];
        state.bestScore = firstRow.score;
        state.bestMaxCombo = firstRow.maxCombo;
        state.bestTime = firstRow.time;
      }
      break;
    }
  });

  physicsAccumulator += dt;
  while (physicsAccumulator >= fixedDt) {
    for (auto& ball : state.balls) {
      ball->physics.prevX = ball->physics.x;
      ball->physics.prevY = ball->physics.y;
    }
    state.playerPaddle.physics.prevX = state.playerPaddle.physics.x;
    state.playerPaddle.physics.prevY = state.playerPaddle.physics.y;

    updatePlayer(state, window.getEvents(), fixedDt);
    updateBalls(state, fixedDt);
    updateBricks(state, fixedDt);
    updatePowerups(state, fixedDt);
    checkCollisions(state);
    updateBackground(state, fixedDt);
    updateUi(state, fixedDt);
    updateParticles(state, fixedDt);
    updateGameFlow(state, fixedDt);
    updateState(state, fixedDt);
    physicsAccumulator -= fixedDt;
  }

  r.update(dt);

  state.frameAggDiff = physicsAccumulator / static_cast<double>(fixedDt);

  for (const auto& musicName : state.musicToPlay) {
    // TODO cross-fade
    if (window.isMusicPlaying()) {
      window.stopMusic();
    }
    window.playMusic(musicName);
  }
  for (const auto& musicPct : state.musicPctToSet) {
    window.setMusicPct(musicPct);
  }
  if (state.stopMusic) {
    if (window.isMusicPlaying()) {
      window.stopMusic();
    }
    state.stopMusic = false;
  }
  state.musicToPlay.clear();
  state.musicPctToSet.clear();

  std::unordered_set<std::string> uniqueSounds(state.soundsToPlay.begin(),
                                               state.soundsToPlay.end());
  for (const auto& soundName : uniqueSounds) {
    window.playSound(soundName);
  }
  state.soundsToPlay.clear();
}

void GameManager::render(int dt) {
  //
  r.renderAll();
}

} // namespace program