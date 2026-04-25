#pragma once

#include "game/actions/AbstractAction.h"

namespace program {
struct Ball;
} // namespace program

namespace program::actions {

enum WallSide { WALL_LEFT, WALL_RIGHT, WALL_TOP };

class DoCollisionBallWall : public AbstractAction {
public:
  Ball* ball;
  WallSide wall;

  DoCollisionBallWall(Ball* ball, WallSide wall) : ball(ball), wall(wall) {}

protected:
  void act() override {}
};

} // namespace program::actions
