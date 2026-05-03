#pragma once

#include "Physics.h"
#include "client/TextState.h"
#include "client/components/EndGameScreen.h"
#include "client/components/InterLevelScreen.h"
#include "game/actions/AbstractAction.h"
#include "lib/sdl2w/Animation.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/L10n.h"
#include "lib/sdl2w/Store.h"
#include "utils/Gauge.hpp"
#include "utils/Timer.hpp"
#include "utils/Transform.hpp"
#include <iterator>
#include <list>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace program {

struct GameParams {
  double ventAirAccel = 0.0005;
  double stuckMotionWindowMs = 16.0; // 2 integration frames if fixedDt is 8
  double stuckMotionMinDistPx = 1.5;
  int numStartingBalls = 5;
  int startingLevel = 0;
  int fixedDt = 8;
  bool playMenuMusic = true;
};

using Line = physics::Line;
using Circle = physics::Circle;
using Rect = physics::Rect;

struct Removable {
  int id;
  bool shouldRemove = false;
};

struct AnimationStorage {
  std::unordered_map<std::string, std::unique_ptr<sdl2w::Animation>> animations;

  sdl2w::Animation& getAnim(sdl2w::Store& store, const std::string& name) {
    if (animations.find(name) == animations.end()) {
      sdl2w::AnimationDefinition def = store.getAnimationDefinition(name);
      animations[name] =
          std::unique_ptr<sdl2w::Animation>(new sdl2w::Animation(def, store));
    }
    return *animations[name];
  }

  void updateAnimations(int dt) {
    for (auto& anim : animations) {
      anim.second->update(dt);
    }
  }
};

struct Particle {
  std::unique_ptr<sdl2w::Animation> animation;
  Timer timer;
  std::string animName;
  std::string text;
  sdl2w::RenderTextParams textParams;
  int x = 0;
  int y = 0;
  int ms = 0;
};

enum PlayerPaddleState {
  PLAYER_PADDLE_SPAWNING,
  PLAYER_PADDLE_NORMAL,
  PLAYER_PADDLE_SHORT,
  PLAYER_PADDLE_FLASH,
  PLAYER_PADDLE_EXPLODE,
};

struct PlayerPaddle : AnimationStorage {
  std::vector<Line> collisionLines{Line(-7, -16, 7, -16)};
  std::vector<Circle> collisionCircles{
      Circle(-7, 16, 32),
      Circle(7, 16, 32),
  };
  Physics physics = {
      .x = 0.,
      .y = 0.,
      .vx = 0.,
      .vy = 0.,
      .ax = 0.,
      .ay = 0.,
      .mass = 1,
      .radius = 8,
      .friction = 0.07,
  };
  PlayerPaddleState paddleState = PLAYER_PADDLE_NORMAL;
  Timer bounceTimer = Timer{100};
  /// While `paddleState == PLAYER_PADDLE_SHORT`, counts down 10s then returns
  /// to normal (same unit as other gameplay timers: ms).
  Timer shortPaddleTimer = Timer{10000};
  TransformLinear transform;
  double acceleration = 0.0105;
  int numBalls = 0;
  bool isBounced = false;
  bool isVisible = true;
  bool leftPressed = false;
  bool rightPressed = false;
  bool isVacuumActive = false;
};

enum BallType {
  BALL_TYPE_NORMAL,
  BALL_TYPE_EXTRA,
  BALL_TYPE_METAL,
};

inline std::string ballTypeToAnimName(BallType ballType) {
  switch (ballType) {
  case BALL_TYPE_NORMAL:
    return "ball_normal";
  case BALL_TYPE_EXTRA:
    return "ball_extra";
  case BALL_TYPE_METAL:
    return "ball_metal";
  }
  return "ball_normal";
}

struct Ball : Removable, AnimationStorage {
  std::list<std::pair<double, double>> previousStates;
  Timer prevStateTimer = Timer{50};
  Timer transitionToPlayerTimer = Timer{500};
  Physics physics;
  BallType ballType = BALL_TYPE_NORMAL;
  double speed = 0.27;
  double angle = 0;
  double angleSpeed = 0.65;
  double radius = 8.;
  double ballTransitionPrevX = 0;
  double ballTransitionPrevY = 0;
  int numPreviousStates = 5;
  bool isStickToPlayer = false;
  bool isHighlighted = false;
  bool isTransitioningToPlayer = false;
  /// If true, the ball moved less than ~5px over the last 200ms (in play only).
  /// Used to break corner-collision oscillation by bouncing off the nearest
  /// edge.
  bool stuckLowMotion = false;
  double stuckMotionAnchorX = 0.;
  double stuckMotionAnchorY = 0.;
  double stuckMotionAccumMs = 0.;
};

enum BrickType {
  BRICK_TYPE_NONE = 0,
  BRICK_TYPE_NORMAL,
  BRICK_TYPE_METAL,
  BRICK_TYPE_METAL_BOMB,
  BRICK_TYPE_POWERUP_EXTRA_BALLS,
  BRICK_TYPE_POWERUP_SHORT_PADDLE,
  BRICK_TYPE_POWERUP_ARMOR,
  BRICK_TYPE_POWERUP_TIMER,
  BRICK_TYPE_POWERUP_METAL_BALLS,
  BRICK_TYPE_BRITTLE,
  BRICK_TYPE_SMALL_SQUARE_NORMAL,
  BRICK_TYPE_SMALL_CIRCLE_NORMAL,
  BRICK_TYPE_SMALL_SQUARE_METAL,
  BRICK_TYPE_SMALL_CIRCLE_METAL,
  BRICK_TYPE_VENT,
  BRICK_TYPE_SMALL_VENT,
  BRICK_TYPE_SMALL_INDESTRUCTIBLE
};

struct VentFan {
  int x = 0;
  int y = 0;
  int direction = 0;
  /// 1 = small vent or right half of a large vent; 2 = left half of a large
  /// vent
  int spanTiles = 1;
};

struct LevelData {
  std::string name = "unset";
  int width = 0;
  int height = 0;
  std::vector<BrickType> bricks;
  std::vector<VentFan> fans;
  int backgroundInd = 0;
};

enum BrickColor {
  BRICK_COLOR_DEFAULT = 0,
  BRICK_COLOR_BLUE,
  BRICK_COLOR_GREEN,
  BRICK_COLOR_PINK,
};

inline std::string brickColorToString(BrickColor brickColor) {
  switch (brickColor) {
  case BRICK_COLOR_DEFAULT:
    return "";
  case BRICK_COLOR_BLUE:
    return "blue";
  case BRICK_COLOR_GREEN:
    return "green";
  case BRICK_COLOR_PINK:
    return "pink";
  }
  return "";
}

struct Brick : Removable, AnimationStorage {
  Physics physics;
  Rect coll;
  Circle circleColl;
  BrickType brickType = BRICK_TYPE_NORMAL;
  TransformLinear transform;
  double angle = 0.;
  bool isDestroyed = false;
  bool isVisible = true;
  bool isArmored = false;
  int w = 58;
  int h = 28;
  int hpInverted = 0;
};

enum PowerupKind { POWERUP_KIND_SHORT_PADDLE };

struct Powerup : Removable, AnimationStorage {
  Physics physics;
  Circle coll;
  PowerupKind kind = POWERUP_KIND_SHORT_PADDLE;
};

/// Sets paddle line/circle collision to match `paddleState` (40px flat top for
/// SHORT, default rounded paddle otherwise).
inline void applyPaddleCollisionProfile(PlayerPaddle& paddle,
                                        PlayerPaddleState paddleState) {
  switch (paddleState) {
  case PLAYER_PADDLE_SHORT:
    paddle.collisionLines = {Line(-20, -4, 20, -4)};
    paddle.collisionCircles = {
        Circle(-20, 16, 20),
        Circle(20, 16, 20),
    };
    break;
  case PLAYER_PADDLE_NORMAL:
  case PLAYER_PADDLE_FLASH:
  case PLAYER_PADDLE_SPAWNING:
  case PLAYER_PADDLE_EXPLODE:
  default:
    paddle.collisionLines = {Line(-7, -16, 7, -16)};
    paddle.collisionCircles = {
        Circle(-7, 16, 32),
        Circle(7, 16, 32),
    };
    break;
  }
}

struct Background {
  std::vector<std::vector<int>> rows;
  double offset = 0.0;
  int terrainIndex = 0;
  int bgWidth = (640 / 24 + 1);
  int bgHeight = (480 / 24 + 2);
  int x = 24 / 2;
  int y = 24 / 2 - 24;
  int spriteSize = 24;
};

struct LevelInfo {
  BrickColor brickColor = BRICK_COLOR_DEFAULT;
  Timer protectionWallsTimer = Timer{10000};
  int levelIndex = 0;
  int brickWidth = 58;
  int brickHeight = 28;
  int brickRadius = 58 / 4;
  int bricksXOffset = 640 / 2 - (8 * 58) / 2;
  int bricksYOffset = 32 - 1;
  int defaultPlayAreaWidth = 640;
  int defaultPlayAreaHeight = 480;
  int defaultPlayAreaXOffset = 0;
  int defaultPlayAreaYOffset = 0;
  int playAreaXOffset = defaultPlayAreaXOffset;
  int playAreaYOffset = defaultPlayAreaYOffset;
  int playAreaWidth = defaultPlayAreaWidth;
  int playAreaHeight = defaultPlayAreaHeight;
  bool preventBallLost = false;
  bool areProtectionWallsEnabled = false;
};

enum FadeDirection {
  FADE_NONE,
  FADE_IN,
  FADE_OUT,
};

struct UiInfo {
  EndGameScreenState endGameScreenState;
  InterLevelScreenState interLevelScreenState;
  BlinkingText startGameText = BlinkingText{
      .text = TRANSLATE("Press button to start."),
  };
  //
  Timer fadeTimer = Timer{1000};
  double fadeBlackScreenAlpha = 0.;
  FadeDirection fadeDirection = FADE_NONE;
  //
  Timer ultraScaleTimer = Timer{750};
  double ultraScale = 1.;
  double ultraScaleMax = 1.5;
  bool ultraScaleIncreasing = true;
  //
  BlinkingText launchBallText = BlinkingText{
      .text = TRANSLATE("Press button for ball."),
  };
  bool showTextIndicatingBallIsReadyToLaunch = false;
  bool showLaunchText = false;
  // Timer showLaunchTextTimer = Timer{500};
  // bool isLaunchTextVisible = false;
  //
  Timer showInterLevelTextTimer = Timer{1000};
  bool isInterLevelTextVisible = false;
  int nextLevelToTransferTo = 0;
  //
  Timer defeatedAutoMenuTimer = Timer{5000.0};
  BlinkingText defeatedRestartPrompt = BlinkingText{
      .text = TRANSLATE("Press button to restart level."),
  };
};

enum ControlState {
  CONTROL_MENU,
  CONTROL_WAITING,
  CONTROL_IN_GAME,
  CONTROL_IN_TRANSITION_TO_NEXT_LEVEL,
  CONTROL_INTER_LEVEL,
  CONTROL_DEFEATED,
  CONTROL_GAME_OVER
};

enum UiState {
  UI_HIDDEN,
  UI_MENU,
  UI_INTER_LEVEL,
  UI_PRE_GAME,
  UI_IN_GAME,
  UI_DEFEATED,
  UI_GAME_OVER
};

struct ActionData {
  std::list<std::unique_ptr<actions::AsyncAction>> sequentialActions;
  std::list<std::unique_ptr<actions::AsyncAction>> sequentialActionsNext;
  std::list<std::unique_ptr<actions::AsyncAction>> insertActions;
  std::vector<std::unique_ptr<actions::AsyncAction>> parallelActions;
};

struct State {
  GameParams params;
  std::unordered_map<std::string, std::unique_ptr<LevelData>> levels;
  std::vector<std::unique_ptr<Ball>> balls;
  std::vector<std::unique_ptr<Brick>> bricks;
  std::vector<VentFan> fans;
  std::vector<std::unique_ptr<Powerup>> powerups;
  std::vector<std::unique_ptr<Particle>> particles;
  std::vector<std::string> soundsToPlay;
  std::vector<std::string> musicToPlay;
  std::vector<int> musicPctToSet;
  sdl2w::Store* store = nullptr;
  ActionData actionData;
  PlayerPaddle playerPaddle;
  Background background;
  LevelInfo levelInfo;
  UiInfo uiInfo;
  std::string brickColor;
  ControlState controlState = CONTROL_WAITING;
  UiState uiState = UI_HIDDEN;

  Gauge juggleGauge = Gauge{
      .max = 1,
      .decayRate = 0.002,
      .value = 0,
  };

  int bestScore = 0;
  int bestMaxCombo = 0;
  int bestTime = 0;
  int score = 0;
  int stageStartScore = 0;
  int stageScore = 0;
  int scoreLevelStart = 0;
  int lastScore = 0;
  int lastMaxCombo = 0;
  int lastComboBonus = 0;
  int interLevelJuggleBonus = 0;
  int interLevelJuggleTier = 0;
  int totalJuggleBonus = 0;
  int juggleGaugeMaxBallsSeen = 1;
  int juggleGaugeCompletedTiers = 0;
  int lastTime = 0;
  int combo = 0;
  int maxCombo = 0;
  int stopwatchMs = 0;
  int retryIndex = 0;
  int numRetries = 0;
  /// Counts restarts after losing all balls (incremented when choosing restart
  /// on the defeated screen), reset when starting a new run from the menu.
  int numContinuesUsed = 0;
  int terrainIndex = 0;
  double frameAggDiff = 0.0;
  bool stopMusic = false;
};

// enqueue action, run it, then wait ms
inline void
enqueueAction(State& state, actions::AbstractAction* action, int ms) {
  auto actionPtr =
      new actions::AsyncAction{std::unique_ptr<actions::AbstractAction>(action),
                               Timer{static_cast<double>(ms), 0}};
  state.actionData.sequentialActionsNext.push_back(
      std::unique_ptr<actions::AsyncAction>(actionPtr));
  // if (actionPtr->action) {
  //   LOG(INFO) << "Enqueued action: " << actionPtr->action->getName() <<
  //   "for"
  //             << ms << "ms" << LOG_ENDL;
  // }
}

inline void
insertAction(State& state, actions::AbstractAction* action, int ms) {
  auto actionPtr =
      new actions::AsyncAction{std::unique_ptr<actions::AbstractAction>(action),
                               Timer{static_cast<double>(ms), 0}};
  state.actionData.insertActions.push_back(
      std::unique_ptr<actions::AsyncAction>(actionPtr));
}

inline void
addParallelAction(State& state, actions::AbstractAction* action, int ms) {
  state.actionData.parallelActions.push_back(
      std::unique_ptr<actions::AsyncAction>(new actions::AsyncAction{
          std::unique_ptr<actions::AbstractAction>(action),
          Timer{static_cast<double>(ms), 0}}));
}

inline void moveSequentialActions(State& state) {
  state.actionData.sequentialActions.splice(
      state.actionData.sequentialActions.end(),
      state.actionData.sequentialActionsNext);
}

inline void updateState(State& state, int dt) {
  moveSequentialActions(state);
  while (!state.actionData.sequentialActions.empty()) {
    auto& delayedActionPtr = state.actionData.sequentialActions.front();
    actions::AsyncAction& delayedAction = *delayedActionPtr;
    if (delayedAction.action.get() != nullptr) {
      delayedAction.action->execute(&state);
      delayedAction.action = nullptr;
      if (!state.actionData.insertActions.empty()) {
        auto afterCurrent =
            std::next(state.actionData.sequentialActions.begin());
        state.actionData.sequentialActions.splice(
            afterCurrent, state.actionData.insertActions);
      }
    }

    timer::update(delayedAction.timer, dt);
    if (timer::isComplete(delayedAction.timer)) {
      bool shouldLoop = delayedAction.timer.duration == 0;
      state.actionData.sequentialActions.pop_front();
      if (shouldLoop) {
        moveSequentialActions(state);
        continue;
      } else {
        break;
      }
    } else {
      break;
    }
  }
  for (unsigned int i = 0; i < state.actionData.parallelActions.size(); i++) {
    auto& delayedActionPtr = state.actionData.parallelActions[i];
    actions::AsyncAction& delayedAction = *delayedActionPtr;
    timer::update(delayedAction.timer, dt);
    if (timer::isComplete(delayedAction.timer)) {
      if (delayedAction.action != nullptr) {
        delayedAction.action->execute(&state);
      }
      state.actionData.parallelActions.erase(
          state.actionData.parallelActions.begin() + i);
      i--;
    }
  }
}

inline std::optional<Ball*> getBallByPtr(State& state, Ball* ballPtr) {
  for (auto& ball : state.balls) {
    if (ball.get() == ballPtr) {
      return ball.get();
    }
  }
  return std::nullopt;
}

inline std::optional<Brick*> getBrickByPtr(State& state, Brick* brickPtr) {
  for (auto& brick : state.bricks) {
    if (brick.get() == brickPtr) {
      return brick.get();
    }
  }
  return std::nullopt;
}

inline std::optional<Powerup*> getPowerupByPtr(State& state,
                                               Powerup* powerupPtr) {
  for (auto& powerup : state.powerups) {
    if (powerup.get() == powerupPtr) {
      return powerup.get();
    }
  }
  return std::nullopt;
}

inline std::optional<Particle*> getParticleByPtr(State& state,
                                                 Particle* particlePtr) {
  for (auto& particle : state.particles) {
    if (particle.get() == particlePtr) {
      return particle.get();
    }
  }
  return std::nullopt;
}

inline double getActiveBallsInPlay(const State& state) {
  int activeBallsInPlay = 0;
  for (const auto& ball : state.balls) {
    if (ball->isStickToPlayer) {
      continue;
    }
    activeBallsInPlay++;
  }
  return activeBallsInPlay;
}

} // namespace program