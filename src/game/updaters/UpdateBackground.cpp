#include "UpdateBackground.h"
#include "game/Data.h"
#include "game/State.h"

namespace program {
void updateBackground(State& state, int dt) {
  double dBgSpriteSize = static_cast<double>(state.background.spriteSize);
  double bgOffset = state.background.offset;
  bgOffset += 0.05 * static_cast<double>(dt);
  if (bgOffset > dBgSpriteSize) {
    bgOffset = bgOffset - static_cast<double>(state.background.spriteSize);
    state.background.rows.pop_back();
    std::vector<int> row;
    generateBackgroundRow(row, state.background.bgWidth);
    state.background.rows.insert(state.background.rows.begin(), 1, row);
  }
  state.background.offset = bgOffset;
}
} // namespace program