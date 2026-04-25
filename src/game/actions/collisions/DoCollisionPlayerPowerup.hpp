#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/setters/SetPlayerPaddleState.hpp"
#include "game/actions/ui/PlaySound.hpp"

namespace program::actions {

class DoCollisionPlayerPowerup : public AbstractAction {
public:
  Powerup* powerup;

  explicit DoCollisionPlayerPowerup(Powerup* powerup) : powerup(powerup) {}

protected:
  void act() override {
    State& localState = *this->state;
    auto po = getPowerupByPtr(localState, powerup);
    if (!po.has_value() || *po == nullptr || (*po)->shouldRemove) {
      return;
    }
    Powerup& p = **po;
    p.shouldRemove = true;
    switch (p.kind) {
    case POWERUP_KIND_SHORT_PADDLE:
      timer::start(localState.playerPaddle.shortPaddleTimer, 10000);
      addParallelAction(localState,
                        new SetPlayerPaddleState(PLAYER_PADDLE_SHORT),
                        0);
      addParallelAction(localState, new PlaySound("powerup_bad"), 0);
      break;
    }
  }
};

} // namespace program::actions
