#include "game/State.h"

namespace program {

long actions::AbstractAction::idCounter = 0;

void actions::AbstractAction::insAct(AbstractAction* action, int ms) {
  if (state == nullptr) {
    return;
  }
  insertAction(*state, action, ms);
}

void actions::AbstractAction::pllAct(AbstractAction* action, int ms) {
  if (state == nullptr) {
    return;
  }
  addParallelAction(*state, action, ms);
}
} // namespace program