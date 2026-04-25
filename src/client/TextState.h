#pragma once

#include "utils/Timer.hpp"
#include <string>

namespace program {

struct BlinkingText {
  Timer timer = Timer{1000};
  std::string text;
  bool isVisible = true;

  void reset() {
    timer::start(timer);
    isVisible = true;
  }

  void update(int dt) {
    timer::update(timer, dt);
    if (timer::isComplete(timer)) {
      isVisible = !isVisible;
      timer::start(timer);
    }
  }
};

struct PulsingText {
  Timer timer = Timer{1000};
  std::string text;
  double maxPulse = 1.2;
  double currentPulse = 1.0;
  bool isPulsingOut = false;
  bool isPulsing = true;

  void reset() {
    timer::start(timer);
    isPulsingOut = false;
    isPulsing = true;
    currentPulse = 1.0;
  }

  double getScale() const { return currentPulse; }

  void update(int dt) {
    if (isPulsing) {
      timer::update(timer, dt);
      double pct =
          isPulsingOut ? 1.0 - timer::getPct(timer) : timer::getPct(timer);
      currentPulse = 1.0 + (maxPulse - 1.0) * pct;
      if (timer::isComplete(timer)) {
        timer::start(timer);
        isPulsingOut = !isPulsingOut;
      }
    }
  }
};

} // namespace program