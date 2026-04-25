#include "CheckCollisions.h"
#include "game/Data.h"
#include "game/State.h"
#include "game/actions/collisions/DoCollisionBallBrick.hpp"
#include "game/actions/collisions/DoCollisionBallLost.hpp"
#include "game/actions/collisions/DoCollisionBallPlayer.hpp"
#include "game/actions/collisions/DoCollisionBallWall.hpp"
#include "game/actions/collisions/DoCollisionPlayerPowerup.hpp"
#include <cmath>
#include <memory>

namespace program {

// Returns true if there is a non-destroyed brick flush against `brick` at the
// given (offsetX, offsetY) from its top-left origin (i.e. one brick-width
// right, one brick-height below, etc.).
static bool hasAdjacentBrick(const std::vector<std::unique_ptr<Brick>>& bricks,
                             const Brick& brick,
                             double offsetX,
                             double offsetY) {
  const double targetX = brick.coll.x + offsetX;
  const double targetY = brick.coll.y + offsetY;
  for (const auto& b : bricks) {
    if (b.get() == &brick || b->isDestroyed) {
      continue;
    }
    if (std::abs(b->coll.x - targetX) < 0.5 &&
        std::abs(b->coll.y - targetY) < 0.5) {
      return true;
    }
  }
  return false;
}

// Adjust a collision direction to account for adjacent bricks so that interior
// faces and corners of a touching brick group are suppressed.  A face that
// borders a live neighbour is interior (return NONE); a corner that borders one
// neighbour on one axis is converted to the remaining face direction.
static physics::CollisionDirection
adjustForAdjacentBricks(physics::CollisionDirection dir,
                        const Brick& brick,
                        const std::vector<std::unique_ptr<Brick>>& bricks) {
  const double bw = brick.coll.w;
  const double bh = brick.coll.h;

  switch (dir) {
  case physics::CollisionDirection::RIGHT:
    if (hasAdjacentBrick(bricks, brick, bw, 0)) {
      return physics::CollisionDirection::NONE;
    }
    break;
  case physics::CollisionDirection::LEFT:
    if (hasAdjacentBrick(bricks, brick, -bw, 0)) {
      return physics::CollisionDirection::NONE;
    }
    break;
  case physics::CollisionDirection::TOP:
    if (hasAdjacentBrick(bricks, brick, 0, -bh)) {
      return physics::CollisionDirection::NONE;
    }
    break;
  case physics::CollisionDirection::BOTTOM:
    if (hasAdjacentBrick(bricks, brick, 0, bh)) {
      return physics::CollisionDirection::NONE;
    }
    break;
  case physics::CollisionDirection::TOP_LEFT: {
    const bool adjLeft = hasAdjacentBrick(bricks, brick, -bw, 0);
    const bool adjTop = hasAdjacentBrick(bricks, brick, 0, -bh);
    if (adjLeft && adjTop) {
      return physics::CollisionDirection::NONE;
    }
    if (adjLeft) {
      return physics::CollisionDirection::TOP;
    }
    if (adjTop) {
      return physics::CollisionDirection::LEFT;
    }
    break;
  }
  case physics::CollisionDirection::TOP_RIGHT: {
    const bool adjRight = hasAdjacentBrick(bricks, brick, bw, 0);
    const bool adjTop = hasAdjacentBrick(bricks, brick, 0, -bh);
    if (adjRight && adjTop) {
      return physics::CollisionDirection::NONE;
    }
    if (adjRight) {
      return physics::CollisionDirection::TOP;
    }
    if (adjTop) {
      return physics::CollisionDirection::RIGHT;
    }
    break;
  }
  case physics::CollisionDirection::BOTTOM_LEFT: {
    const bool adjLeft = hasAdjacentBrick(bricks, brick, -bw, 0);
    const bool adjBottom = hasAdjacentBrick(bricks, brick, 0, bh);
    if (adjLeft && adjBottom) {
      return physics::CollisionDirection::NONE;
    }
    if (adjLeft) {
      return physics::CollisionDirection::BOTTOM;
    }
    if (adjBottom) {
      return physics::CollisionDirection::LEFT;
    }
    break;
  }
  case physics::CollisionDirection::BOTTOM_RIGHT: {
    const bool adjRight = hasAdjacentBrick(bricks, brick, bw, 0);
    const bool adjBottom = hasAdjacentBrick(bricks, brick, 0, bh);
    if (adjRight && adjBottom) {
      return physics::CollisionDirection::NONE;
    }
    if (adjRight) {
      return physics::CollisionDirection::BOTTOM;
    }
    if (adjBottom) {
      return physics::CollisionDirection::RIGHT;
    }
    break;
  }
  default:
    break;
  }
  return dir;
}

// Normal from collidesCircleCircle points from brick center toward ball center
// (same coords as Physics.cpp). Maps to CollisionDirection for DoCollisionBallBrick.
static physics::CollisionDirection
collisionDirectionFromBrickBallNormal(double nx, double ny) {
  if (std::abs(nx) >= std::abs(ny)) {
    return nx > 0.0 ? physics::CollisionDirection::RIGHT
                    : physics::CollisionDirection::LEFT;
  }
  return ny > 0.0 ? physics::CollisionDirection::BOTTOM
                  : physics::CollisionDirection::TOP;
}

static Rect paddlePowerupPickupRect(const PlayerPaddle& paddle) {
  const double halfW =
      paddle.paddleState == PLAYER_PADDLE_SHORT ? 20.0 : 42.0;
  const double w = halfW * 2.0;
  return Rect{paddle.physics.x - halfW, paddle.physics.y - 6.0, w, 12.0};
}

static Circle brickCircleForSmallCircleCollision(const Brick& brick) {
  if (brick.circleColl.r > 0.0) {
    return brick.circleColl;
  }
  const double cx = brick.coll.x + brick.coll.w * 0.5;
  const double cy = brick.coll.y + brick.coll.h * 0.5;
  const double rr =
      0.5 * std::min(brick.coll.w, brick.coll.h);
  return Circle(cx, cy, rr);
}

void checkCollisions(State& state) {
  auto& paddle = state.playerPaddle;
  // const double W = state.levelInfo.playAreaWidth;
  const double H = state.levelInfo.playAreaHeight;

  for (auto& ballPtr : state.balls) {
    Ball& ball = *ballPtr;

    if (ball.isStickToPlayer || ball.isTransitioningToPlayer ||
        ball.isHighlighted) {
      continue;
    }

    const double r = ball.radius;

    // Ball vs Bricks
    for (auto& brickPtr : state.bricks) {
      Brick& brick = *brickPtr;
      if (brick.isDestroyed || isVentBrick(brick.brickType)) {
        continue;
      }

      if (isSmallCircleBrick(brick.brickType)) {
        Circle ballCirc(ball.physics.x, ball.physics.y, r);
        Circle brickCirc = brickCircleForSmallCircleCollision(brick);
        auto hit = physics::collidesCircleCircle(ballCirc, brickCirc);
        if (hit) {
          physics::CollisionDirection dir = collisionDirectionFromBrickBallNormal(
              hit->nx, hit->ny);
          addParallelAction(
              state,
              new actions::DoCollisionBallBrick(&ball, &brick, dir),
              0);
        }
      } else {
        physics::CollisionDirection dir = physics::collidesCircleRect(
            Circle(ball.physics.x, ball.physics.y, r), brick.coll);
        dir = adjustForAdjacentBricks(dir, brick, state.bricks);
        if (dir != physics::CollisionDirection::NONE) {
          addParallelAction(
              state,
              new actions::DoCollisionBallBrick(&ball, &brick, dir),
              0);
        }
      }
    }

    // Ball vs Player paddle
    std::optional<physics::CircleLineHit> bestHit;
    if (ball.physics.y > state.playerPaddle.physics.y - 64. &&
        state.playerPaddle.isVisible) {
      for (const auto& line : paddle.collisionLines) {
        physics::Line worldLine{paddle.physics.x + line.x1,
                                paddle.physics.y + line.y1,
                                paddle.physics.x + line.x2,
                                paddle.physics.y + line.y2};
        auto hit = physics::collidesCircleLine(
            Circle(ball.physics.x, ball.physics.y, ball.radius), worldLine);
        if (hit && (!bestHit || hit->penetration > bestHit->penetration)) {
          bestHit = hit;
        }
      }
      // Collision circles: only applicable when ball is more than 7px from
      // paddle center on x-axis AND above the bottom of the paddle sprite.
      const double dxFromPaddle = ball.physics.x - paddle.physics.x;
      const bool circleApplicable =
          !paddle.collisionCircles.empty() && std::abs(dxFromPaddle) > 7.0 &&
          ball.physics.y < paddle.physics.y + 16.0;
      if (circleApplicable) {
        for (const auto& circ : paddle.collisionCircles) {
          Circle worldCirc{
              paddle.physics.x + circ.x, paddle.physics.y + circ.y, circ.r};
          auto hit = physics::collidesCircleCircle(
              Circle(ball.physics.x, ball.physics.y, ball.radius), worldCirc);
          if (hit && (!bestHit || hit->penetration > bestHit->penetration)) {
            bestHit = hit;
          }
        }
      }

      if (bestHit) {
        addParallelAction(
            state,
            new actions::DoCollisionBallPlayer(
                &ball, bestHit->nx, bestHit->ny, bestHit->penetration),
            0);
      }
    }

    // Ball vs Walls (left, right, top)
    // if (ball.physics.x - r < 0) {
    //   addParallelAction(
    //       state,
    //       std::unique_ptr<actions::AbstractAction>(
    //           new actions::DoCollisionBallWall(&ball, actions::WALL_LEFT)),
    //       0);
    // } else if (ball.physics.x + r > W) {
    //   addParallelAction(
    //       state,
    //       std::unique_ptr<actions::AbstractAction>(
    //           new actions::DoCollisionBallWall(&ball,
    //           actions::WALL_RIGHT)),
    //       0);
    // } else if (ball.physics.y - r < 0) {
    //   addParallelAction(
    //       state,
    //       std::unique_ptr<actions::AbstractAction>(
    //           new actions::DoCollisionBallWall(&ball, actions::WALL_TOP)),
    //       0);
    // }

    // Ball lost (past bottom edge)
    if (ball.physics.y > H + 16.) {
      addParallelAction(state, new actions::DoCollisionBallLost(&ball), 0);
    }
  }

  // Powerup vs Player paddle
  for (auto& powerupPtr : state.powerups) {
    Powerup& powerup = *powerupPtr;
    if (powerup.shouldRemove) {
      continue;
    }

    // Sync collision circle position from physics
    powerup.coll.x = powerup.physics.x;
    powerup.coll.y = powerup.physics.y;

    const Rect paddleRect = paddlePowerupPickupRect(paddle);
    physics::CollisionDirection dir =
        physics::collidesCircleRect(powerup.coll, paddleRect);
    if (dir != physics::CollisionDirection::NONE) {
      addParallelAction(state, new actions::DoCollisionPlayerPowerup(&powerup), 0);
    }
  }
}

} // namespace program
