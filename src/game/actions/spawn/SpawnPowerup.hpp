#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {

namespace actions {

class SpawnPowerup : public AbstractAction {
  double x;
  double y;
  PowerupKind kind;

  void act() override {
    State& localState = *this->state;
    auto pu = std::unique_ptr<Powerup>(new Powerup{});
    pu->kind = kind;
    pu->physics.x = x;
    pu->physics.y = y;
    pu->physics.prevX = x;
    pu->physics.prevY = y;
    pu->physics.vx = 0.;
    pu->physics.vy = 0.09;
    pu->coll = Circle{x, y, 12.};
    localState.powerups.push_back(std::move(pu));
  }

public:
  SpawnPowerup(double x, double y, PowerupKind kindA = POWERUP_KIND_SHORT_PADDLE)
      : x(x), y(y), kind(kindA) {}
};

} // namespace actions

} // namespace program
