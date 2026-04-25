

#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {

namespace actions {

class SpawnBall : public AbstractAction {
  double x;
  double y;
  double vx;
  double vy;
  double speed;
  BallType ballType;
  bool isStickToPlayer;
  void act() override {
    State& localState = *this->state;

    LOG(INFO) << "Spawning ball at " << x << ", " << y << LOG_ENDL;
    localState.balls.push_back(std::unique_ptr<Ball>(new Ball{}));
    auto& ball = *localState.balls.back();
    ball.physics.x = x;
    ball.physics.y = y;
    ball.physics.prevX = x;
    ball.physics.prevY = y;
    ball.physics.vx = vx;
    ball.physics.vy = vy;
    ball.speed = speed;
    ball.ballType = ballType;
    ball.radius = 8;
    ball.isTransitioningToPlayer = false;
    ball.isHighlighted = false;
    ball.isStickToPlayer = isStickToPlayer;
  }

public:
  SpawnBall(double x,
            double y,
            double vx,
            double vy,
            double speed,
            BallType ballType = BALL_TYPE_NORMAL,
            bool isStickToPlayer = false)
      : x(x),
        y(y),
        vx(vx),
        vy(vy),
        speed(speed),
        ballType(ballType),
        isStickToPlayer(isStickToPlayer) {}
};

} // namespace actions

} // namespace program