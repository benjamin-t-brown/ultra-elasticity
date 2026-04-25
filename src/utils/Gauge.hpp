#pragma once

namespace program {

struct Gauge {
  double max = 0;
  double decayRate = 0;
  double value = 0;
};

namespace gauge {

inline void reset(Gauge& gauge) {
  //
  gauge.value = 0;
}

inline void fill(Gauge& gauge, const double amount) {
  //
  gauge.value += amount;
  if (gauge.value < 0) {
    gauge.value = 0;
  }
}

inline void setMax(Gauge& gauge, const double max) {
  //
  gauge.max = max;
}

inline double getPct(const Gauge& gauge) {
  //
  return gauge.value / gauge.max;
}

inline double getValue(const Gauge& gauge) {
  //
  return gauge.value;
}

inline double getMax(const Gauge& gauge) {
  //
  return gauge.max;
}

inline void update(Gauge& gauge, const double dt) {
  if (gauge.value > 0) {
    gauge.value -= gauge.decayRate * dt;
    if (gauge.value < 0) {
      gauge.value = 0;
    }
  }
}

} // namespace gauge

} // namespace program