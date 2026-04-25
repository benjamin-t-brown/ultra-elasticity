#pragma once
#include "game/Data.h"
#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "game/actions/ui/PlaySound.hpp"
#include <cmath>
#include <memory>

namespace program::actions {

class ExplodeBomb : public AbstractAction {
  double collX;
  double collY;

  void act() override {
    State& s = *state;
    constexpr double kAdjTolerance = 0.5;
    auto axisOverlaps = [](double aStart, double aLen, double bStart,
                           double bLen) {
      return aStart < (bStart + bLen) && bStart < (aStart + aLen);
    };
    Brick* brick = nullptr;
    for (auto& b : s.bricks) {
      if (!b->isDestroyed && std::abs(b->coll.x - collX) < kAdjTolerance &&
          std::abs(b->coll.y - collY) < kAdjTolerance) {
        brick = b.get();
        break;
      }
    }
    if (!brick) {
      return;
    }

    brick->isDestroyed = true;
    s.combo++;
    s.score += 1 * s.combo;
    addParallelAction(s, new PlaySound("explosion"), 0);

    for (auto& b : s.bricks) {
      if (b->isDestroyed || b.get() == brick) {
        continue;
      }
      const bool isLeftNeighbor =
          std::abs((b->coll.x + b->coll.w) - brick->coll.x) < kAdjTolerance &&
          axisOverlaps(b->coll.y, b->coll.h, brick->coll.y, brick->coll.h);
      const bool isRightNeighbor =
          std::abs(b->coll.x - (brick->coll.x + brick->coll.w)) <
              kAdjTolerance &&
          axisOverlaps(b->coll.y, b->coll.h, brick->coll.y, brick->coll.h);
      const bool isTopNeighbor =
          std::abs((b->coll.y + b->coll.h) - brick->coll.y) < kAdjTolerance &&
          axisOverlaps(b->coll.x, b->coll.w, brick->coll.x, brick->coll.w);
      const bool isBottomNeighbor =
          std::abs(b->coll.y - (brick->coll.y + brick->coll.h)) <
              kAdjTolerance &&
          axisOverlaps(b->coll.x, b->coll.w, brick->coll.x, brick->coll.w);
      if ((isLeftNeighbor || isRightNeighbor || isTopNeighbor ||
           isBottomNeighbor) &&
          isMetalBrick(b->brickType)) {
        addParallelAction(s, new ExplodeBomb(b->coll.x, b->coll.y), 250);
      }
    }
  }

public:
  ExplodeBomb(double collX, double collY) : collX(collX), collY(collY) {}
};

} // namespace program::actions
