#include "UpdateEntities.h"
#include "client/UiRenderer.h"
#include "game/Data.h"
#include "game/Physics.h"
#include "game/State.h"
#include "game/actions/level/ControlProtectionWalls.hpp"
#include "game/actions/notify/NotifyScore.hpp"
#include "game/actions/setters/SetPlayerPaddleState.hpp"
#include "game/actions/spawn/SpawnParticle.hpp"
#include "game/actions/transitions/TransitionToGameOver.hpp"
#include "game/actions/transitions/TransitionToInterLevel.hpp"
#include "game/actions/transitions/TransitionToLevelLive.hpp"
#include "game/actions/transitions/TransitionToLevelLoss.hpp"
#include "game/actions/transitions/TransitionToMenu.hpp"
#include "utils/Gauge.hpp"
#include <array>
#include <cmath>

namespace program {

namespace {

void updateBallStuckMotion(Ball& ball, int dt, const State& state) {
  if (ball.isStickToPlayer || ball.isHighlighted) {
    ball.stuckLowMotion = false;
    ball.stuckMotionAccumMs = 0.;
    return;
  }
  const double dblDt = static_cast<double>(dt);
  if (ball.stuckMotionAccumMs <= 0.0) {
    ball.stuckMotionAnchorX = ball.physics.x;
    ball.stuckMotionAnchorY = ball.physics.y;
  }
  ball.stuckMotionAccumMs += dblDt;
  if (ball.stuckMotionAccumMs >= state.params.stuckMotionWindowMs) {
    const double dx = ball.physics.x - ball.stuckMotionAnchorX;
    const double dy = ball.physics.y - ball.stuckMotionAnchorY;
    ball.stuckLowMotion =
        std::hypot(dx, dy) < state.params.stuckMotionMinDistPx;
    ball.stuckMotionAnchorX = ball.physics.x;
    ball.stuckMotionAnchorY = ball.physics.y;
    ball.stuckMotionAccumMs = 0.;
  }
}

void applyVentAirForcesToBall(Ball& ball, const State& state) {
  if (state.controlState != CONTROL_IN_GAME) {
    return;
  }
  if (state.fans.empty()) {
    return;
  }
  // One force per unique vent direction (30–36) per frame; overlapping twin
  // fans on two-tile vents use the same direction and must not stack.
  std::array<bool, 7> didApplyForDirectionId{};
  for (const auto& fan : state.fans) {
    physics::Rect air{};
    double headingDeg = 0.;
    getVentAirZoneForFan(state, fan, air, headingDeg);
    if (physics::collidesCircleRect(
            physics::Circle{ball.physics.x, ball.physics.y, ball.radius},
            air) == physics::CollisionDirection::NONE) {
      continue;
    }
    int dirBucket = fan.direction - 30;
    if (dirBucket < 0 || dirBucket > 6) {
      dirBucket = 0;
    }
    if (didApplyForDirectionId[static_cast<size_t>(dirBucket)]) {
      continue;
    }
    didApplyForDirectionId[static_cast<size_t>(dirBucket)] = true;
    physics::applyForce(ball.physics, headingDeg, state.params.ventAirAccel);
  }
}
} // namespace

void updateUi(State& state, int dt) {
  state.uiInfo.startGameText.update(dt);

  if (state.uiInfo.fadeDirection != FADE_NONE) {
    timer::update(state.uiInfo.fadeTimer, dt);
    state.uiInfo.fadeBlackScreenAlpha =
        state.uiInfo.fadeDirection == FADE_IN
            ? 1. - timer::getPct(state.uiInfo.fadeTimer)
            : timer::getPct(state.uiInfo.fadeTimer);
    if (timer::isComplete(state.uiInfo.fadeTimer)) {
      state.uiInfo.fadeBlackScreenAlpha =
          state.uiInfo.fadeDirection == FADE_IN ? 0. : 1.;
      state.uiInfo.fadeDirection = FADE_NONE;
      timer::start(state.uiInfo.fadeTimer);
    }
  }

  // if (state.uiInfo.showLaunchText) {
  // }
  if (state.balls.size() == 0 && state.controlState == CONTROL_IN_GAME) {
    state.uiInfo.launchBallText.update(dt);
  } else {
    state.uiInfo.launchBallText.isVisible = true;
  }

  if (state.controlState == CONTROL_DEFEATED) {
    state.uiInfo.defeatedRestartPrompt.update(dt);
  }

  timer::update(state.uiInfo.ultraScaleTimer, dt);
  state.uiInfo.ultraScale =
      state.uiInfo.ultraScaleIncreasing
          ? 1. + (state.uiInfo.ultraScaleMax - 1.) *
                     timer::getPct(state.uiInfo.ultraScaleTimer)
          : (state.uiInfo.ultraScaleMax) -
                (state.uiInfo.ultraScaleMax - 1.) *
                    timer::getPct(state.uiInfo.ultraScaleTimer);
  if (timer::isComplete(state.uiInfo.ultraScaleTimer)) {
    state.uiInfo.ultraScaleIncreasing = !state.uiInfo.ultraScaleIncreasing;
    timer::start(state.uiInfo.ultraScaleTimer);
  }
}

void updateBalls(State& state, int dt) {
  for (int i = 0; i < static_cast<int>(state.balls.size()); i++) {
    auto& ball = state.balls[i];

    if (ball->shouldRemove) {
      state.balls.erase(state.balls.begin() + i);
      i--;
      continue;
    }
    ball->updateAnimations(dt);
    if (ball->isTransitioningToPlayer) {
      timer::update(ball->transitionToPlayerTimer, dt);
      double pct = timer::getPct(ball->transitionToPlayerTimer);
      ball->physics.x =
          ball->ballTransitionPrevX +
          (state.playerPaddle.physics.x - ball->ballTransitionPrevX) * pct;
      ball->physics.y =
          ball->ballTransitionPrevY +
          (state.playerPaddle.physics.y - ball->ballTransitionPrevY) * pct;
      if (timer::isComplete(ball->transitionToPlayerTimer)) {
        ball->isTransitioningToPlayer = false;
      }
    } else {
      if (ball->isStickToPlayer) {
        ball->physics.x = state.playerPaddle.physics.x;
      } else if (!ball->isHighlighted) {
        if (abs(ball->physics.vy) < 0.005) {
          physics::applyForce(ball->physics, 180., 0.0002);
        }
        applyVentAirForcesToBall(*ball, state);
      }
      physics::updatePhysics(ball->physics, dt);

      ball->angle += ball->angleSpeed * dt;
      if (ball->angle > 360.) {
        ball->angle -= 360.;
      } else if (ball->angle < 0) {
        ball->angle += 360.;
      }

      double x = ball->physics.x;
      double y = ball->physics.y;
      bool isSticky = ball->isStickToPlayer;
      double ballHalfSize = ball->radius;
      const double W =
          state.levelInfo.playAreaWidth + state.levelInfo.playAreaXOffset;
      const double H =
          state.levelInfo.playAreaHeight + state.levelInfo.playAreaYOffset;
      if (x > W - ballHalfSize) {
        ball->physics.x = W - ballHalfSize;
        if (!isSticky) {
          ball->physics.vx = -ball->physics.vx;
          ball->angleSpeed = -ball->angleSpeed;
          ball->physics.x = W - ballHalfSize;
          if (state.levelInfo.areProtectionWallsEnabled) {
            enqueueAction(state, new actions::PlaySound("brick_hit1"), 0);
          }
        }
      } else if (x < ballHalfSize + state.levelInfo.playAreaXOffset) {
        ball->physics.x = ballHalfSize + state.levelInfo.playAreaXOffset;
        if (!isSticky) {
          ball->physics.vx = -ball->physics.vx;
          ball->angleSpeed = -ball->angleSpeed;
          if (state.levelInfo.areProtectionWallsEnabled) {
            enqueueAction(state, new actions::PlaySound("brick_hit1"), 0);
          }
        }
      }
      if (y > H - ballHalfSize) {
        if (state.levelInfo.preventBallLost ||
            state.levelInfo.areProtectionWallsEnabled) {
          ball->physics.y = H - ballHalfSize;
          ball->physics.vy = -ball->physics.vy;
          ball->angleSpeed = -ball->angleSpeed;
          if (state.levelInfo.areProtectionWallsEnabled) {
            enqueueAction(state, new actions::PlaySound("brick_hit1"), 0);
          }
        }
      } else if (y < ballHalfSize + state.levelInfo.playAreaYOffset) {
        ball->physics.y = ballHalfSize + state.levelInfo.playAreaYOffset;
        ball->physics.vy = -ball->physics.vy;
        ball->angleSpeed = -ball->angleSpeed;
        if (state.levelInfo.areProtectionWallsEnabled) {
          enqueueAction(state, new actions::PlaySound("brick_hit1"), 0);
        }
      }

      updateBallStuckMotion(*ball, dt, state);
    }

    timer::update(ball->prevStateTimer, dt);
    if (timer::isComplete(ball->prevStateTimer)) {
      ball->previousStates.push_back({ball->physics.x, ball->physics.y});
      timer::start(ball->prevStateTimer);
      if (static_cast<int>(ball->previousStates.size()) >
          ball->numPreviousStates) {
        ball->previousStates.erase(ball->previousStates.begin());
      }
    }
  }
}

void updateBricks(State& state, int dt) {
  for (int i = 0; i < static_cast<int>(state.bricks.size()); i++) {
    auto& brick = state.bricks[i];
    brick->updateAnimations(dt);
    if (!brick->isDestroyed) {
      brick->transform.update(dt);
    }
  }
}

void updatePowerups(State& state, int dt) {
  if (state.controlState != CONTROL_IN_GAME) {
    return;
  }
  const double H = state.levelInfo.playAreaHeight;
  constexpr double kCullBelow = 24.;
  for (int i = 0; i < static_cast<int>(state.powerups.size()); i++) {
    auto& pu = state.powerups[i];
    if (pu->shouldRemove) {
      state.powerups.erase(state.powerups.begin() + i);
      i--;
      continue;
    }
    pu->updateAnimations(dt);
    pu->physics.prevX = pu->physics.x;
    pu->physics.prevY = pu->physics.y;
    pu->physics.y += pu->physics.vy * static_cast<double>(dt);
    if (pu->physics.y > H + kCullBelow) {
      state.powerups.erase(state.powerups.begin() + i);
      i--;
    }
  }
}

void updateParticles(State& state, int dt) {
  for (int i = 0; i < static_cast<int>(state.particles.size()); i++) {
    auto& p = state.particles[i];
    if (p->animation) {
      p->animation->update(dt);
    }
    timer::update(p->timer, dt);
    if (timer::isComplete(p->timer)) {
      state.particles.erase(state.particles.begin() + i);
      i--;
    }
  }
}

void updateGameFlow(State& state, int dt) {
  if (state.controlState == CONTROL_IN_GAME && state.uiState == UI_IN_GAME &&
      state.uiInfo.fadeDirection == FADE_NONE) {
    state.stopwatchMs += dt;
  }

  if (state.controlState == CONTROL_IN_GAME) {
    const double activeBallsInPlay = getActiveBallsInPlay(state);
    const double fillRate = 0.0004;
    // const double decayRate = 0.0005;
    if (activeBallsInPlay > state.juggleGaugeMaxBallsSeen) {
      state.juggleGaugeMaxBallsSeen = activeBallsInPlay;
    }
    gauge::setMax(state.juggleGauge, std::max(1., activeBallsInPlay));

    auto pct = gauge::getPct(state.juggleGauge);

    if (activeBallsInPlay == 0 || pct >= 1.0) {
      // disable for now
      // gauge::fill(state.juggleGauge, -decayRate);
    } else {
      gauge::fill(state.juggleGauge,
                  fillRate + fillRate * ((activeBallsInPlay - 1.) * 1.75));
    }

    bool hasMetalBall = false;
    for (const auto& ball : state.balls) {
      if (ball->ballType == BALL_TYPE_METAL) {
        hasMetalBall = true;
        break;
      }
    }

    int intVal = static_cast<int>(gauge::getValue(state.juggleGauge));
    if (intVal > state.juggleGaugeCompletedTiers) {
      state.juggleGaugeCompletedTiers = intVal;
      addParallelAction(state, new actions::PlaySound("juggle_bonus"), 0);
      addParallelAction(
          state,
          new actions::SpawnParticle(
              TRANSLATE("Juggle Bonus") + std::string(" x ") +
                  std::to_string(state.juggleGaugeCompletedTiers + 1),
              state.playerPaddle.physics.x,
              state.playerPaddle.physics.y - 70,
              2000,
              sdl2w::RenderTextParams{
                  .fontName = "default",
                  .fontSize = sdl2w::TextSize::TEXT_SIZE_16,
                  .color = COLOR_YELLOW,
                  .centered = true,
              }),
          0);
    }

    if (state.levelInfo.areProtectionWallsEnabled) {
      timer::update(state.levelInfo.protectionWallsTimer, dt);
      if (timer::isComplete(state.levelInfo.protectionWallsTimer)) {
        enqueueAction(state, new actions::ControlProtectionWalls(false), 0);
      }
    }

    if (state.playerPaddle.paddleState == PLAYER_PADDLE_SHORT) {
      timer::update(state.playerPaddle.shortPaddleTimer, dt);
      if (timer::isComplete(state.playerPaddle.shortPaddleTimer)) {
        enqueueAction(
            state, new actions::SetPlayerPaddleState(PLAYER_PADDLE_NORMAL), 0);
      }
    }

    bool allRelevantBricksDestroyed = true;
    for (const auto& brick : state.bricks) {
      if (!brick->isDestroyed) {
        if (hasMetalBall) {
          if (isVentBrick(brick->brickType) ||
              isIndestructibleBrick(brick->brickType)) {
            continue;
          }
        } else {
          if (isMetalBrickNoBomb(brick->brickType) ||
              isVentBrick(brick->brickType) ||
              isIndestructibleBrick(brick->brickType)) {
            continue;
          }
        }
        allRelevantBricksDestroyed = false;
        break;
      }
    }
    if (allRelevantBricksDestroyed) {
      LOG(INFO) << "All relevant bricks destroyed, transitioning level"
                << LOG_ENDL;
      // Preserve an in-progress streak when the level ends before the ball
      // returns to the paddle.
      if (state.combo > state.maxCombo) {
        state.maxCombo = state.combo;
      }
      const int highestTier = std::max(0, state.juggleGaugeCompletedTiers);
      const int stageScore = std::max(0, state.stageScore);
      const int juggleBonus = stageScore * highestTier;
      int nextLevel = state.levelInfo.levelIndex + 1;
      state.interLevelJuggleBonus = juggleBonus;
      state.interLevelJuggleTier = highestTier;
      if (hasLevel(state, nextLevel)) {
        if (nextLevel % 3 == 0) {
          state.levelInfo.areProtectionWallsEnabled = false;
          state.playerPaddle.paddleState = PLAYER_PADDLE_NORMAL;
          enqueueAction(
              state, new actions::TransitionToInterLevel(nextLevel), 0);
        } else {
          enqueueAction(state,
                        new actions::TransitionToLevelLive(
                            state.levelInfo.levelIndex + 1),
                        0);
        }
      } else {
        state.levelInfo.areProtectionWallsEnabled = false;
        state.playerPaddle.paddleState = PLAYER_PADDLE_NORMAL;
        enqueueAction(state, new actions::TransitionToGameOver(), 0);
      }
    }
    if (state.balls.empty()) {
      if (state.playerPaddle.numBalls > 0) {
        state.uiInfo.showTextIndicatingBallIsReadyToLaunch = true;
      } else {
        LOG(INFO) << "No balls left, transitioning to level loss" << LOG_ENDL;
        state.levelInfo.areProtectionWallsEnabled = false;
        state.playerPaddle.paddleState = PLAYER_PADDLE_NORMAL;
        enqueueAction(state, new actions::TransitionToLevelLoss(), 0);
      }
    }
  }

  if (state.controlState == CONTROL_DEFEATED) {
    timer::update(state.uiInfo.defeatedAutoMenuTimer, dt);
    if (timer::isComplete(state.uiInfo.defeatedAutoMenuTimer)) {
      state.controlState = CONTROL_WAITING;
      enqueueAction(state, new actions::NotifyGameCompleted(0, 0, 0), 0);
      enqueueAction(state, new actions::TransitionToMenu(true), 0);
    }
  }
}

void updatePlayerPaddle(State& state, int dt) {
  if (state.playerPaddle.numBalls > 0) {
    state.playerPaddle.numBalls--;
  }
}

} // namespace program