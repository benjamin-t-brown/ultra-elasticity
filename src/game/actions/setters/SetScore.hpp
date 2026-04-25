#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"

namespace program {

namespace actions {

class SetScore : public AbstractAction {
  bool addMaxComboBonus = false;

  void act() override {
    State& localState = *this->state;
    localState.score = localState.stageStartScore + localState.stageScore +
                       localState.stageScore * localState.juggleGaugeCompletedTiers;
    if (addMaxComboBonus) {
      localState.score += localState.maxCombo * 1000;
    }
  }

public:
  SetScore(bool addMaxComboBonus = false)
      : addMaxComboBonus(addMaxComboBonus) {}
};

} // namespace actions

} // namespace program