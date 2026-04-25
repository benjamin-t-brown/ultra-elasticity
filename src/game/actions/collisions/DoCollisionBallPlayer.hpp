#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/ui/PlaySound.hpp"
#include "lib/sdl2w/L10n.h"
#include <sstream>
#include <string>

namespace program::actions {

class DoCollisionBallPlayer : public AbstractAction {
  Ball* ball;
  double nx;
  double ny; // surface normal pointing toward ball center (unit vector)
  double penetration; // how far ball overlaps the line

  void act() override {
    State& localState = *this->state;
    auto ballOpt = getBallByPtr(localState, ball);
    if (!ballOpt) {
      return;
    };
    Ball& b = **ballOpt;

    // Push ball out of paddle surface along the normal
    b.physics.x += nx * penetration;
    b.physics.y += ny * penetration;

    // Reflect velocity across the normal, but only if moving into the surface.
    // dot < 0 means velocity opposes the normal (ball moving toward surface).
    double dot = b.physics.vx * nx + b.physics.vy * ny;
    if (dot < 0.0) {
      b.physics.vx -= 2.0 * dot * nx;
      b.physics.vy -= 2.0 * dot * ny;
    }
    if (localState.combo > 2) {
      std::stringstream ss;
      ss << (TRANSLATE("Combo ")) << localState.combo;
      localState.particles.push_back(std::make_unique<Particle>(Particle{
          .timer = Timer{900, 0},
          .text = ss.str(),
          .textParams = {
              .fontName = "default",
              .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
              .color = {255, 255, 255, 255},
          },
          .x = static_cast<int>(localState.playerPaddle.physics.x),
          .y = static_cast<int>(localState.playerPaddle.physics.y - 48),
          .ms = 900,
      }));
    }
    const int comboAtPaddleHit = localState.combo;
    if (comboAtPaddleHit > localState.maxCombo) {
      localState.maxCombo = comboAtPaddleHit;
    }
    if (comboAtPaddleHit > 2) {
      const std::string comboSound =
          comboAtPaddleHit < 5 ? "combo1"
          : comboAtPaddleHit < 10 ? "combo2"
                                  : "combo3";
      addParallelAction(localState, new PlaySound(comboSound), 0);
    }
    localState.combo = 0;
    localState.playerPaddle.isBounced = true;
    timer::start(localState.playerPaddle.bounceTimer);

    // Transfer paddle horizontal velocity to ball on contact, scaled by how
    // much of a top hit it is (abs(ny) ≈ 1 for top, ≈ 0 for side)
    b.physics.vx += localState.playerPaddle.physics.vx * 0.4 * abs(nx);

    addParallelAction(localState, new PlaySound("paddle_hit"), 0);
  }

public:
  DoCollisionBallPlayer(Ball* ball, double nx, double ny, double penetration)
      : ball(ball), nx(nx), ny(ny), penetration(penetration) {}
};

} // namespace program::actions
