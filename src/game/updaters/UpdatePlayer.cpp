#include "UpdatePlayer.h"
#include "client/Keys.hpp"
#include "lib/sdl2w/Events.h"

namespace program {
bool hasControl(State& state) {
  return state.controlState == CONTROL_IN_GAME ||
         state.controlState == CONTROL_IN_TRANSITION_TO_NEXT_LEVEL;
}

void updatePlayer(State& state, sdl2w::Events& events, int dt) {
  auto& paddle = state.playerPaddle;
  if (hasControl(state)) {
    if (events.isKeyPressed(getLeftKey())) {
      paddle.leftPressed = true;
      paddle.rightPressed = false;
    } else if (events.isKeyPressed(getRightKey())) {
      paddle.rightPressed = true;
      paddle.leftPressed = false;
    } else {
      paddle.leftPressed = false;
      paddle.rightPressed = false;
    }
  }

  paddle.updateAnimations(dt);
  if (paddle.transform.isRunning) {
    paddle.transform.update(dt);
    return;
  }

  const double ACCELERATION = paddle.acceleration;
  if (paddle.leftPressed) {
    physics::applyForce(paddle.physics, 270., ACCELERATION);
  } else if (paddle.rightPressed) {
    physics::applyForce(paddle.physics, 90., ACCELERATION);
  }

  physics::updatePhysics(paddle.physics, dt);
  if (paddle.isBounced) {
    timer::update(paddle.bounceTimer, dt);
    if (timer::isComplete(paddle.bounceTimer)) {
      paddle.isBounced = false;
      timer::start(paddle.bounceTimer);
    }
  }

  if (paddle.physics.x < 16) {
    paddle.physics.x = 16;
  } else if (paddle.physics.x > 640 - 16) {
    paddle.physics.x = 640 - 16;
  }
}
} // namespace program