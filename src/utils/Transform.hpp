#pragma once

#include "Timer.hpp"
#include <utility>

namespace program {

struct TransformLinear {
  Timer timer;
  double startX = 0.;
  double startY = 0.;
  double endX = 0.;
  double endY = 0.;
  bool isRunning = false;

  void start(double duration = 0.) {
    timer::start(timer, duration == 0. ? timer.duration : duration);
    isRunning = true;
  }

  void setStartPos(double x, double y) {
    startX = x;
    startY = y;
  }

  void setEndPos(double x, double y) {
    endX = x;
    endY = y;
  }

  std::pair<double, double> getPos() const {
    double progress = timer::getPct(timer);
    return {startX + (endX - startX) * progress,
            startY + (endY - startY) * progress};
  }

  void update(int dt) {
    if (!isRunning) {
      return;
    }
    timer::update(timer, dt);
    if (timer::isComplete(timer)) {
      isRunning = false;
    }
  }
};

} // namespace program