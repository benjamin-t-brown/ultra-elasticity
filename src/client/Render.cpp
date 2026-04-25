#include "Render.h"
#include "UiRenderer.h"
#include "game/Data.h"
#include "game/State.h"
#include "lib/sdl2w/Animation.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Window.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>
#include <memory>

namespace program {

using sdl2w::Animation;
using sdl2w::RenderableParams;
using sdl2w::RenderableParamsEx;

constexpr double TILE_SCALE = 1;

enum VentFanDirection {
  VENT_FAN_DIRECTION_UP = 30,
  VENT_FAN_DIRECTION_RIGHT = 31,
  VENT_FAN_DIRECTION_LEFT = 32,
  VENT_FAN_DIRECTION_UPRIGHT = 33,
  VENT_FAN_DIRECTION_UPLEFT = 34,
  VENT_FAN_DIRECTION_DOWNRIGHT = 35,
  VENT_FAN_DIRECTION_DOWNLEFT = 36,
};

std::string fanDirectionToAnimName(int directionId) {
  switch (directionId) {
  case VENT_FAN_DIRECTION_UP:
    return "fan_to_up";
  case VENT_FAN_DIRECTION_RIGHT:
    return "fan_to_right";
  case VENT_FAN_DIRECTION_LEFT:
    return "fan_to_left";
  case VENT_FAN_DIRECTION_UPLEFT:
    return "fan_to_upleft";
  case VENT_FAN_DIRECTION_UPRIGHT:
    return "fan_to_upright";
  case VENT_FAN_DIRECTION_DOWNLEFT:
    return "fan_to_downleft";
  case VENT_FAN_DIRECTION_DOWNRIGHT:
    return "fan_to_downright";
  default:
    return "fan_to_up";
  }
}
struct FanOffset {
  int x = 0;
  int y = 0;
  int xDir = 0;
  int yDir = 0;
};
FanOffset fanDirectionToOffset(int directionId) {
  switch (directionId) {
  case VENT_FAN_DIRECTION_UP:
    return {0 - 33, -60, 16, 0};
  case VENT_FAN_DIRECTION_RIGHT:
    return {29, -26, 0, 18};
  case VENT_FAN_DIRECTION_LEFT:
    return {-80, -26, 0, 18};
  case VENT_FAN_DIRECTION_UPLEFT:
    return {-80, -60, -1, -1};
  case VENT_FAN_DIRECTION_UPRIGHT:
    return {80, -60, 1, -1};
  case VENT_FAN_DIRECTION_DOWNLEFT:
    return {-80, +60, -1, 1};
  case VENT_FAN_DIRECTION_DOWNRIGHT:
    return {80, +60, 1, 1};
  default:
    return {0, -60, 0, -1};
  }
}

void drawVentAirZoneOutline(sdl2w::Draw& d, const physics::Rect& r) {
  const int x0 = static_cast<int>(std::lround(r.x));
  const int y0 = static_cast<int>(std::lround(r.y));
  const int x1 = static_cast<int>(std::lround(r.x + r.w));
  const int y1 = static_cast<int>(std::lround(r.y + r.h));
  const SDL_Color red{255, 0, 0, 255};
  constexpr int kLineW = 1;
  d.drawLine({x0, y0}, {x1, y0}, kLineW, red);
  d.drawLine({x1, y0}, {x1, y1}, kLineW, red);
  d.drawLine({x1, y1}, {x0, y1}, kLineW, red);
  d.drawLine({x0, y1}, {x0, y0}, kLineW, red);
}

Render::Render(sdl2w::Window& windowA) : window(windowA), ui(new Ui(*this)) {}

void Render::setup(State& state) {
  animations.clear();
  statePtr = &state;
  ui->setup();
}

sdl2w::Animation& Render::getAnim(const std::string& name) {
  return getAnim(animations, name);
}

sdl2w::Animation& Render::getAnim(
    std::unordered_map<std::string, std::unique_ptr<sdl2w::Animation>>& anims,
    const std::string& name) {

  if (anims.find(name) == anims.end()) {
    sdl2w::AnimationDefinition def =
        window.getStore().getAnimationDefinition(name);
    anims[name] =
        std::unique_ptr<Animation>(new Animation(def, window.getStore()));
  }
  return *anims[name];
}

void Render::update(int dt) {
  ui->update(dt);
  for (auto& anim : animations) {
    anim.second->update(dt);
  }
}

void Render::renderBackground() {
  const State& state = getState();
  sdl2w::Draw& d = window.getDraw();
  const auto& rows = state.background.rows;
  for (int i = 0; i < static_cast<int>(rows.size()); i++) {
    for (int j = 0; j < static_cast<int>(rows[0].size()); j++) {
      int bgInd = rows[i][j];
      std::stringstream ss;
      ss << "terrain_" << state.background.terrainIndex << "_" << bgInd;
      d.drawSprite(
          //
          window.getStore().getSprite(ss.str()),
          RenderableParams{
              //
              .scale = {TILE_SCALE, TILE_SCALE},
              .x = state.background.x + j * state.background.spriteSize,
              .y = state.background.y + i * state.background.spriteSize +
                   static_cast<int>(state.background.offset),
          });
    }
  }
}

void Render::renderBalls() {
  const State& state = getState();
  sdl2w::Draw& d = window.getDraw();
  for (auto& ball : state.balls) {
    Animation& anim = ball->isHighlighted
                          ? ball->getAnim(window.getStore(), "ball_highlighted")
                          : ball->getAnim(window.getStore(),
                                          ballTypeToAnimName(ball->ballType));

    int alpha = 256 / 2;
    for (auto it = ball->previousStates.rbegin();
         it != ball->previousStates.rend();
         ++it) {
      d.setGlobalAlpha(alpha);
      d.drawAnimation(anim,
                      RenderableParamsEx{
                          .scale = {TILE_SCALE, TILE_SCALE},
                          .angleDeg = ball->angle,
                          .x = static_cast<int>(it->first),
                          .y = static_cast<int>(it->second),
                          .centered = true,
                      });
      d.setGlobalAlpha(255);
      alpha -= 20;
    }
    auto [rx, ry] = ball->physics.lerpPos(state.frameAggDiff);
    d.drawAnimation(anim,
                    RenderableParamsEx{
                        .scale = {TILE_SCALE, TILE_SCALE},
                        .angleDeg = ball->angle,
                        .x = static_cast<int>(rx),
                        .y = static_cast<int>(ry),
                        .centered = true,
                    });
  }
}

void Render::renderParticles() {
  const State& state = getState();
  sdl2w::Draw& d = window.getDraw();
  for (auto& particle : state.particles) {
    if (particle->animation) {
      d.drawAnimation(*particle->animation,
                      RenderableParamsEx{
                          .scale = {TILE_SCALE, TILE_SCALE},
                          .x = particle->x,
                          .y = particle->y,
                          .centered = true,
                      });
    }
    if (!particle->text.empty()) {
      d.drawText(particle->text,
                 sdl2w::RenderTextParams{
                     .fontName = particle->textParams.fontName,
                     .fontSize = particle->textParams.fontSize,
                     .x = particle->x,
                     .y = particle->y,
                     .color = particle->textParams.color,
                     .centered = true,
                 });
    }
  }
}

void Render::renderBricks() {
  const State& state = getState();
  sdl2w::Draw& d = window.getDraw();
  for (auto& brick : state.bricks) {
    if (!brick->isVisible) {
      continue;
    }

    std::stringstream ss;
    std::stringstream ssDestroyed;
    switch (brick->brickType) {
    case BRICK_TYPE_NORMAL:
      ss << "brick_normal";
      if (state.levelInfo.brickColor != BRICK_COLOR_DEFAULT) {
        ss << "_" << brickColorToString(state.levelInfo.brickColor);
      }
      ssDestroyed << ss.str() << "_destroyed";
      break;
    case BRICK_TYPE_METAL:
      ss << "brick_metal";
      ssDestroyed << ss.str() << "_destroyed";
      break;
    case BRICK_TYPE_METAL_BOMB:
      ss << "brick_metalBomb";
      ssDestroyed << ss.str() << "_destroyed";
      break;
    case BRICK_TYPE_POWERUP_EXTRA_BALLS:
      ss << "brick_powerup";
      ssDestroyed << "brick_normal_destroyed";
      break;
    case BRICK_TYPE_POWERUP_SHORT_PADDLE:
      ss << "brick_powerup_bad";
      ssDestroyed << "brick_normal_destroyed";
      break;
    case BRICK_TYPE_POWERUP_ARMOR:
      ss << "brick_powerup_armor";
      ssDestroyed << "brick_normal_destroyed";
      break;
    case BRICK_TYPE_POWERUP_TIMER:
      ss << "brick_powerup_time";
      ssDestroyed << "brick_normal_destroyed";
      break;
    case BRICK_TYPE_POWERUP_METAL_BALLS:
      ss << "brick_powerup_metal";
      ssDestroyed << "brick_normal_destroyed";
      break;
    case BRICK_TYPE_BRITTLE:
      ss << "brick_brittle"
         << ((brick->hpInverted < 4) ? brick->hpInverted : 3);
      ssDestroyed << "brick_brittle_destroyed";
      break;
    case BRICK_TYPE_SMALL_SQUARE_NORMAL:
      ss << "brick_normal_square";
      ssDestroyed << "brick_small_square_destroyed";
      break;
    case BRICK_TYPE_SMALL_CIRCLE_NORMAL:
      ss << "brick_normal_circle";
      ssDestroyed << "brick_small_circle_destroyed";
      break;
    case BRICK_TYPE_SMALL_SQUARE_METAL:
      ss << "brick_metal_square";
      ssDestroyed << "brick_small_expl";
      break;
    case BRICK_TYPE_SMALL_CIRCLE_METAL:
      ss << "brick_metal_circle";
      ssDestroyed << "brick_small_expl";
      break;
    case BRICK_TYPE_VENT:
      ss << "brick_vent";
      break;
    case BRICK_TYPE_SMALL_VENT:
      ss << "brick_small_vent";
      break;
    case BRICK_TYPE_SMALL_INDESTRUCTIBLE:
      ss << "brick_small_indestructible";
      break;
    default:
      continue;
    }
    auto brickX = brick->physics.x;
    auto brickY = brick->physics.y;
    auto brickAngle = 0.;
    if (brick->transform.isRunning) {
      auto [tx, ty] = brick->transform.getPos();
      brickX = tx;
      brickY = ty;
      auto pct = timer::getPct(brick->transform.timer);
      brickAngle = pct * (360 * 2);
    }
    auto& anim = brick->getAnim(
        window.getStore(), brick->isDestroyed ? ssDestroyed.str() : ss.str());
    bool localIsVentBrick = isVentBrick(brick->brickType);
    if (localIsVentBrick) {
      d.setGlobalAlpha(128);
    }
    d.drawAnimation(anim,
                    RenderableParamsEx{
                        .scale = {TILE_SCALE, TILE_SCALE},
                        .angleDeg = brickAngle,
                        .x = static_cast<int>(brickX),
                        .y = static_cast<int>(brickY),
                        .centered = false,
                    });
    if (localIsVentBrick) {
      d.setGlobalAlpha(255);
    }
  }
}

void Render::renderVentFans() {
  const State& state = getState();
  sdl2w::Draw& d = window.getDraw();
  if (state.controlState != CONTROL_IN_GAME) {
    return;
  }

  const int levelTileWidth = state.levelInfo.brickWidth / 2;
  for (const auto& fan : state.fans) {
    const int fanX = state.levelInfo.bricksXOffset + fan.x * levelTileWidth;
    const int fanY =
        state.levelInfo.bricksYOffset + fan.y * state.levelInfo.brickHeight;
    auto [ventFanOffsetX, ventFanOffsetY, venFanDirOffsetX, venFanDirOffsetY] =
        fanDirectionToOffset(fan.direction);
    const int fanXParticle = fanX + ventFanOffsetX;
    const int fanYParticle = fanY + ventFanOffsetY;
    auto animName = fanDirectionToAnimName(fan.direction);
    auto& anim1 = getAnim(animName);
    auto& anim2 = getAnim(animName + "_2");
    d.setGlobalAlpha(128);
    d.drawAnimation(anim1,
                    RenderableParams{
                        .scale = {TILE_SCALE, TILE_SCALE},
                        .x = fanXParticle,
                        .y = fanYParticle,
                        .centered = false,
                    });
    d.drawAnimation(anim2,
                    RenderableParams{
                        .scale = {TILE_SCALE, TILE_SCALE},
                        .x = fanXParticle + venFanDirOffsetX * 1,
                        .y = fanYParticle + venFanDirOffsetY * 1,
                        .centered = false,
                    });
    d.setGlobalAlpha(255);
    // debug
    // {
    //   physics::Rect airZone{};
    //   double airHeading = 0.;
    //   getVentAirZoneForFan(state, fan, airZone, airHeading);
    //   (void)airHeading;
    //   drawVentAirZoneOutline(d, airZone);
    // }
  }
}

void Render::renderPowerups() {
  State& mutState = getStateMutable();
  const State& state = getState();
  if (state.controlState != CONTROL_IN_GAME) {
    return;
  }
  sdl2w::Draw& d = window.getDraw();
  for (auto& puPtr : mutState.powerups) {
    Powerup& pu = *puPtr;
    if (pu.shouldRemove) {
      continue;
    }
    auto& anim = pu.getAnim(window.getStore(), "powerup_bad");
    auto [rx, ry] = pu.physics.lerpPos(state.frameAggDiff);
    d.drawAnimation(anim,
                    RenderableParams{
                        .scale = {TILE_SCALE, TILE_SCALE},
                        .x = static_cast<int>(rx),
                        .y = static_cast<int>(ry),
                        .centered = true,
                    });
  }
}

void Render::renderPlayerPaddle() {
  const State& state = getState();
  if (!state.playerPaddle.isVisible) {
    return;
  }

  sdl2w::Draw& d = window.getDraw();
  std::stringstream shipAnimBase;
  std::stringstream paddleAnimBase;
  shipAnimBase << "player2";
  paddleAnimBase << "paddle2";
  switch (state.playerPaddle.paddleState) {
  case PLAYER_PADDLE_SPAWNING: {
    shipAnimBase << "_intro";
    paddleAnimBase << "_intro";
    break;
  }
  case PLAYER_PADDLE_NORMAL: {
    if (state.playerPaddle.leftPressed) {
      shipAnimBase << "_left";
    } else if (state.playerPaddle.rightPressed) {
      shipAnimBase << "_right";
    } else {
      shipAnimBase << "_up";
    }
    paddleAnimBase << "_normal";
    break;
  }
  case PLAYER_PADDLE_SHORT:
    if (state.playerPaddle.leftPressed) {
      shipAnimBase << "_left";
    } else if (state.playerPaddle.rightPressed) {
      shipAnimBase << "_right";
    } else {
      shipAnimBase << "_up";
    }
    paddleAnimBase << "_short";
    break;
  case PLAYER_PADDLE_FLASH:
    shipAnimBase << "_flash";
    paddleAnimBase << "_normal";
    break;
  case PLAYER_PADDLE_EXPLODE:
    shipAnimBase << "_explode";
    paddleAnimBase << "_invisible";
    break;
  }

  auto& shipAnim = getStateMutable().playerPaddle.getAnim(window.getStore(),
                                                          shipAnimBase.str());
  auto& paddleAnim = getStateMutable().playerPaddle.getAnim(
      window.getStore(), paddleAnimBase.str());
  auto& sparkleAnim = getStateMutable().playerPaddle.getAnim(window.getStore(),
                                                             "paddle2_sparkle");
  auto [rx, ry] = state.playerPaddle.physics.lerpPos(state.frameAggDiff);
  int px = static_cast<int>(rx);
  int py = static_cast<int>(ry);
  if (state.playerPaddle.transform.isRunning) {
    auto [tx, ty] = state.playerPaddle.transform.getPos();
    px = static_cast<int>(tx);
    py = static_cast<int>(ty);
  }
  d.drawAnimation(shipAnim,
                  RenderableParams{
                      .scale = {TILE_SCALE, TILE_SCALE},
                      .x = px,
                      .y = py,
                      .centered = true,
                  });
  d.drawAnimation(paddleAnim,
                  RenderableParams{
                      .scale = {TILE_SCALE, TILE_SCALE},
                      .x = px,
                      .y = py,
                      .centered = true,
                  });
  if (state.playerPaddle.paddleState == PLAYER_PADDLE_SPAWNING) {
    d.drawAnimation(sparkleAnim,
                    RenderableParams{
                        .scale = {TILE_SCALE, TILE_SCALE},
                        .x = px,
                        .y = py,
                        .centered = true,
                    });
  }

  // debug collision lines and circles
  // for (auto& line : state.playerPaddle.collisionLines) {
  //   d.drawLine({static_cast<int>(rx + line.x1), static_cast<int>(ry +
  //   line.y1)},
  //              {static_cast<int>(rx + line.x2), static_cast<int>(ry +
  //              line.y2)}, 2, {255, 0, 0, 255});
  // }
  // for (auto& circle : state.playerPaddle.collisionCircles) {
  //   d.drawCircle(static_cast<int>(rx + circle.x),
  //                static_cast<int>(ry + circle.y),
  //                static_cast<int>(circle.r),
  //                {100, 100, 255, 255},
  //                false);
  // }
}

void Render::renderFadeBlackScreen() {
  auto [w, h] = window.getDraw().getRenderSize();
  int alpha = static_cast<int>(255 * getState().uiInfo.fadeBlackScreenAlpha);
  uint8_t uintAlpha = alpha;
  window.getDraw().drawRect(0, 0, w, h, {0, 0, 0, uintAlpha});
}

void Render::renderAll() {
  renderBackground();
  renderBricks();
  renderVentFans();
  renderPlayerPaddle();
  renderPowerups();
  renderBalls();
  renderParticles();
  ui->render();
  renderFadeBlackScreen();
  ui->renderAboveFade();
}
} // namespace program