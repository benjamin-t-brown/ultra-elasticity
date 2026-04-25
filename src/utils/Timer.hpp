#pragma once

namespace program {

struct Timer {
  double duration = 0;
  double t = 0;
};

namespace timer {

inline void start(Timer& timer, const double duration) {
  timer.t = 0;
  timer.duration = duration;
}

inline void start(Timer& timer) {
  timer.t = 0;
}

inline bool isComplete(const Timer& timer) { return timer.t >= timer.duration; }

inline bool isRunning(const Timer& timer) { return timer.t < timer.duration; }

inline double getPct(const Timer& timer) {
  return timer.t / timer.duration;
}

inline void update(Timer& timer, const double dt) {
  if (isRunning(timer)) {
    timer.t += dt;
    if (timer.t > timer.duration) {
      timer.t = timer.duration;
    }
  }
}
} // namespace timer

} // namespace program