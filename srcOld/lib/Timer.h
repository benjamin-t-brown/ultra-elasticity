#pragma once

#include <functional>

// Simple ms-based timer classes to replace SDL2Wrapper::Timer/BoolTimer/FuncTimer/Gauge.
// Old API used milliseconds as the time unit (same as new).

class Timer {
protected:
  int totalMs;
  int remainingMs;
  bool done;

public:
  Timer(int ms) : totalMs(ms), remainingMs(ms), done(false) {}
  virtual ~Timer() = default;

  virtual void update(int dt) {
    if (done) return;
    remainingMs -= dt;
    if (remainingMs <= 0) {
      remainingMs = 0;
      done = true;
      onDone();
    }
  }

  virtual void onDone() {}

  bool shouldRemove() const { return done; }

  double getPctComplete() const {
    if (totalMs <= 0) return 1.0;
    return 1.0 - (static_cast<double>(remainingMs) / static_cast<double>(totalMs));
  }
};

class BoolTimer : public Timer {
  bool& ref;

public:
  BoolTimer(int ms, bool& refA) : Timer(ms), ref(refA) {}

  void onDone() override { ref = false; }
};

class FuncTimer : public Timer {
  std::function<void()> cb;

public:
  FuncTimer(int ms, std::function<void()> cbA) : Timer(ms), cb(cbA) {}

  void onDone() override {
    if (cb) cb();
  }
};

// Simple fill gauge: increments each call to fill(), triggers when full.
// Replaces SDL2Wrapper::Gauge(window, maxCount).
class Gauge {
  int current;
  int max;

public:
  Gauge(int maxA) : current(0), max(maxA) {}

  void fill() { current++; }
  void empty() { current = 0; }
  bool isFull() const { return current >= max; }
};
