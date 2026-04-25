#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {
namespace actions {

class ResetStageScore : public AbstractAction {
  void act() override {
    State& localState = *this->state;
    localState.juggleGaugeCompletedTiers = 0;
    localState.juggleGauge.value = 0;
    localState.juggleGaugeMaxBallsSeen = 1;
    localState.stageStartScore = localState.score;
    localState.stageScore = 0;
  }
};

} // namespace actions
} // namespace program