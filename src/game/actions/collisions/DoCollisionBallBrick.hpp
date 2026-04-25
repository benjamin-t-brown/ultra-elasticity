#pragma once

#include "game/Data.h"
#include "game/Physics.h"
#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/collisions/ExplodeBomb.hpp"
#include "game/actions/level/ControlProtectionWalls.hpp"
#include "game/actions/spawn/SpawnBall.hpp"
#include "game/actions/spawn/SpawnPowerup.hpp"
#include "game/actions/ui/PlaySound.hpp"
#include <cmath>
#include <cstdlib>
#include <memory>

namespace program::actions {

class DoCollisionBallBrick : public AbstractAction {
  Ball* ball;
  Brick* brick;
  physics::CollisionDirection direction;

  // Unit normals pointing out of the brick at each corner (45° bisector).
  // Do not use (ball - corner): when the ball sits in the brick's interior
  // octant from that vertex, that vector points into the brick, so separation
  // leaves the ball inside and velocity can stay tangential — next integration
  // returns to the same overlap.
  static void cornerOutwardNormal(physics::CollisionDirection corner,
                                  double& nx,
                                  double& ny) {
    constexpr double h = 0.70710678118654752440; // 1/sqrt(2)
    switch (corner) {
    case physics::CollisionDirection::TOP_LEFT:
      nx = -h;
      ny = -h;
      break;
    case physics::CollisionDirection::TOP_RIGHT:
      nx = h;
      ny = -h;
      break;
    case physics::CollisionDirection::BOTTOM_LEFT:
      nx = -h;
      ny = h;
      break;
    case physics::CollisionDirection::BOTTOM_RIGHT:
      nx = h;
      ny = h;
      break;
    default:
      nx = 0.0;
      ny = -1.0;
      break;
    }
  }

  // Reflect ball off a corner point: position-correct and reflect velocity.
  void reflectCorner(Ball& b,
                     double cornerX,
                     double cornerY,
                     physics::CollisionDirection corner) {
    double nx, ny;
    cornerOutwardNormal(corner, nx, ny);

    // Position correction: place ball outside corner and add tiny separation
    // bias to avoid immediate re-collision due to float jitter.
    const double separationBias = 0.25;
    b.physics.x = cornerX + nx * (b.radius + separationBias);
    b.physics.y = cornerY + ny * (b.radius + separationBias);
    // Reflect velocity if moving into the brick (against outward normal).
    double dot = b.physics.vx * nx + b.physics.vy * ny;
    if (dot < 0.0) {
      b.physics.vx -= 2.0 * dot * nx;
      b.physics.vy -= 2.0 * dot * ny;
    }
  }

  // When corner handling oscillates, snap/reflect like the nearest AABB face.
  static void reflectClosestEdge(Ball& b, const Rect& r) {
    const double bx = b.physics.x;
    const double by = b.physics.y;
    const double dTop = std::abs(by - r.y);
    const double dBottom = std::abs(by - (r.y + r.h));
    const double dLeft = std::abs(bx - r.x);
    const double dRight = std::abs(bx - (r.x + r.w));

    physics::CollisionDirection edge = physics::CollisionDirection::TOP;
    double best = dTop;
    if (dBottom < best) {
      best = dBottom;
      edge = physics::CollisionDirection::BOTTOM;
    }
    if (dLeft < best) {
      best = dLeft;
      edge = physics::CollisionDirection::LEFT;
    }
    if (dRight < best) {
      edge = physics::CollisionDirection::RIGHT;
    }

    switch (edge) {
    case physics::CollisionDirection::TOP:
      b.physics.y = r.y - b.radius;
      if (b.physics.vy > 0.0) {
        b.physics.vy = -b.physics.vy;
      }
      break;
    case physics::CollisionDirection::BOTTOM:
      b.physics.y = r.y + r.h + b.radius;
      if (b.physics.vy < 0.0) {
        b.physics.vy = -b.physics.vy;
      }
      break;
    case physics::CollisionDirection::LEFT:
      b.physics.x = r.x - b.radius;
      if (b.physics.vx > 0.0) {
        b.physics.vx = -b.physics.vx;
      }
      break;
    case physics::CollisionDirection::RIGHT:
      b.physics.x = r.x + r.w + b.radius;
      if (b.physics.vx < 0.0) {
        b.physics.vx = -b.physics.vx;
      }
      break;
    default:
      break;
    }
  }

  void resetStuckMotionAfterUnstick(Ball& b) {
    b.stuckLowMotion = false;
    b.stuckMotionAccumMs = 0.;
  }

  void playSound(const std::string& sound) {
    auto& localState = *state;
    addParallelAction(localState, new PlaySound(sound), 0);
  }

  void act() override {
    auto ballOpt = getBallByPtr(*state, ball);
    if (!ballOpt) {
      return;
    };
    auto brickOpt = getBrickByPtr(*state, brick);
    if (!brickOpt) {
      return;
    };

    Ball& b = **ballOpt;
    Brick& br = **brickOpt;
    const Rect& r = br.coll;
    auto& localState = *state;
    auto markBrickDestroyed = [&]() {
      if (!br.isDestroyed) {
        br.isDestroyed = true;
        localState.combo++;
        const int scoreDelta = 1 * localState.combo;
        // localState.score += scoreDelta;
        localState.stageScore += scoreDelta;
      }
    };

    bool isMetalBall = b.ballType == BALL_TYPE_METAL;
    bool brickIsMetalType = isMetalBrick(br.brickType);
    br.hpInverted++;

    // Metal balls pass through non-metal bricks without reflecting.
    // All other cases reflect normally.
    bool didReflect = false;
    if (!isMetalBall || brickIsMetalType ||
        br.brickType == BRICK_TYPE_SMALL_INDESTRUCTIBLE) {
      switch (direction) {
      case physics::CollisionDirection::TOP:
        b.physics.y = r.y - b.radius;
        if (b.physics.vy > 0.0) {
          b.physics.vy = -b.physics.vy;
          didReflect = true;
        }
        break;
      case physics::CollisionDirection::BOTTOM:
        b.physics.y = r.y + r.h + b.radius;
        if (b.physics.vy < 0.0) {
          b.physics.vy = -b.physics.vy;
          didReflect = true;
        }
        break;
      case physics::CollisionDirection::LEFT:
        b.physics.x = r.x - b.radius;
        if (b.physics.vx > 0.0) {
          b.physics.vx = -b.physics.vx;
          didReflect = true;
        }
        break;
      case physics::CollisionDirection::RIGHT:
        b.physics.x = r.x + r.w + b.radius;
        if (b.physics.vx < 0.0) {
          b.physics.vx = -b.physics.vx;
          didReflect = true;
        }
        break;
      case physics::CollisionDirection::TOP_LEFT:
        if (!b.isStickToPlayer && b.stuckLowMotion) {
          reflectClosestEdge(b, r);
          resetStuckMotionAfterUnstick(b);
        } else {
          reflectCorner(b, r.x, r.y, physics::CollisionDirection::TOP_LEFT);
        }
        didReflect = true;
        break;
      case physics::CollisionDirection::TOP_RIGHT:
        if (!b.isStickToPlayer && b.stuckLowMotion) {
          reflectClosestEdge(b, r);
          resetStuckMotionAfterUnstick(b);
        } else {
          reflectCorner(
              b, r.x + r.w, r.y, physics::CollisionDirection::TOP_RIGHT);
        }
        didReflect = true;
        break;
      case physics::CollisionDirection::BOTTOM_LEFT:
        if (!b.isStickToPlayer && b.stuckLowMotion) {
          reflectClosestEdge(b, r);
          resetStuckMotionAfterUnstick(b);
        } else {
          reflectCorner(
              b, r.x, r.y + r.h, physics::CollisionDirection::BOTTOM_LEFT);
        }
        didReflect = true;
        break;
      case physics::CollisionDirection::BOTTOM_RIGHT:
        if (!b.isStickToPlayer && b.stuckLowMotion) {
          reflectClosestEdge(b, r);
          resetStuckMotionAfterUnstick(b);
        } else {
          reflectCorner(
              b, r.x + r.w, r.y + r.h, physics::CollisionDirection::BOTTOM_RIGHT);
        }
        didReflect = true;
        break;
      default:
        break;
      }
    }
    if (didReflect) {
      b.angleSpeed = -b.angleSpeed;
    }

    switch (br.brickType) {
    case BRICK_TYPE_METAL:
      if (isMetalBall) {
        markBrickDestroyed();
        playSound("brick_hit2");
        playSound("explosion");
      } else {
        playSound("metal_hit2");
      }
      break;
    case BRICK_TYPE_METAL_BOMB:
      addParallelAction(
          localState, new actions::ExplodeBomb(br.coll.x, br.coll.y), 0);
      break;
    case BRICK_TYPE_POWERUP_METAL_BALLS:
      markBrickDestroyed();
      for (auto& activeBall : localState.balls) {
        activeBall->ballType = BALL_TYPE_METAL;
      }
      playSound("brick_hit1");
      addParallelAction(localState, new PlaySound("ball_metalify"), 0);
      break;
    case BRICK_TYPE_POWERUP_SHORT_PADDLE:
      markBrickDestroyed();
      playSound("brick_hit1");
      playSound("inter_level");
      addParallelAction(localState,
                        new actions::SpawnPowerup(r.x + r.w * 0.5,
                                                  r.y + r.h * 0.5,
                                                  POWERUP_KIND_SHORT_PADDLE),
                        0);
      break;
    case BRICK_TYPE_POWERUP_EXTRA_BALLS: {
      markBrickDestroyed();
      playSound("brick_hit1");
      for (int i = 0; i < 2; i++) {
        double angle =
            (rand() / static_cast<double>(RAND_MAX)) * 2.0 * LOCAL_PI;
        addParallelAction(localState,
                          new actions::SpawnBall(b.physics.x,
                                                 b.physics.y,
                                                 b.speed * std::cos(angle),
                                                 b.speed * std::sin(angle),
                                                 b.speed,
                                                 BALL_TYPE_EXTRA),
                          0);
      }
      addParallelAction(localState, new PlaySound("extra_balls"), 0);
      break;
    }
    case BRICK_TYPE_BRITTLE: {
      if (br.hpInverted >= 4) {
        markBrickDestroyed();
        playSound("brick_brittle_expl");
      } else {
        playSound("brick_brittle_hit");
      }

      break;
    }
    case BRICK_TYPE_POWERUP_ARMOR: {
      markBrickDestroyed();
      enqueueAction(localState, new actions::ControlProtectionWalls(true), 0);
      break;
    }
    case BRICK_TYPE_SMALL_SQUARE_METAL:
    case BRICK_TYPE_SMALL_CIRCLE_METAL: {
      if (isMetalBall) {
        markBrickDestroyed();
      }
      playSound(isMetalBall ? "brick_hit2" : "metal_hit2");
      break;
    }
    case BRICK_TYPE_SMALL_INDESTRUCTIBLE: {
      playSound("metal_hit2");
      break;
    }
    case BRICK_TYPE_POWERUP_TIMER:
    case BRICK_TYPE_SMALL_SQUARE_NORMAL:
    case BRICK_TYPE_SMALL_CIRCLE_NORMAL:
    case BRICK_TYPE_NORMAL: {
      markBrickDestroyed();
      playSound("brick_hit1");
      break;
    case BRICK_TYPE_SMALL_VENT:
    case BRICK_TYPE_VENT:
    case BRICK_TYPE_NONE:
      break;
    }
    }
  }

public:
  DoCollisionBallBrick(Ball* ball,
                       Brick* brick,
                       physics::CollisionDirection direction)
      : ball(ball), brick(brick), direction(direction) {}
};

} // namespace program::actions
