#pragma once

#include <SDL2/SDL_rect.h>
#include <optional>
#include <utility>
#include <vector>

namespace program {
constexpr double LOCAL_PI = 3.14159265358979323846;

struct Physics {
  double x = 0.;
  double y = 0.;
  double prevX = 0.;
  double prevY = 0.;
  double vx = 0.;
  double vy = 0.;
  double ax = 0.;
  double ay = 0.;
  double mass = 1.;
  double radius = 32.;
  double friction = 0.0;

  std::pair<double, double> lerpPos(double alpha) const {
    return {prevX + (x - prevX) * alpha, prevY + (y - prevY) * alpha};
  }
};

enum HeadingTurnDirection {
  NONE,
  LEFT,
  RIGHT,
};

struct Heading {
  double angle = 0.;
  double rotationRate = 0.25;
  HeadingTurnDirection turnDirection = NONE;
};

namespace physics {

class Circle {
public:
  double x = 0.;
  double y = 0.;
  double r = 0.;
  Circle() = default;
  Circle(double xA, double yA, double rA) : x(xA), y(yA), r(rA) {}
};

class Rect {
public:
  double x = 0.;
  double y = 0.;
  double w = 0.;
  double h = 0.;
  Rect() = default;
  Rect(double xA, double yA, double wA, double hA)
      : x(xA), y(yA), w(wA), h(hA) {}
};

class Line {
public:
  double x1 = 0.;
  double y1 = 0.;
  double x2 = 0.;
  double y2 = 0.;
  Line() = default;
  Line(double x1A, double y1A, double x2A, double y2A)
      : x1(x1A), y1(y1A), x2(x2A), y2(y2A) {}
};

enum CollisionDirection {
  NONE,
  TOP,
  BOTTOM,
  LEFT,
  RIGHT,
  TOP_LEFT,
  TOP_RIGHT,
  BOTTOM_LEFT,
  BOTTOM_RIGHT
};
double getAngleTowards(std::pair<double, double> p1,
                       std::pair<double, double> p2);
double getAngleTowards(std::pair<double, double> p1,
                       std::pair<double, double> p2);
double getAngleTowards(const Physics& p1, std::pair<double, double> p2);

std::pair<double, double> getPos(const Physics& physics);
void applyForce(Physics& physics, double headingDeg, double acc);
void updateHeading(Heading& heading, const int dt);
void updatePhysics(Physics& physics, const int dt);
void updatePhysics(Physics& physics,
                   const int dt,
                   std::vector<Rect>& obstacles);
struct CircleLineHit {
  double nx, ny;      // surface normal pointing toward ball center (unit vector)
  double penetration; // r - dist: how far circle overlaps the line
};

CollisionDirection collidesCircleRect(const Circle& c, const Rect& r);
std::optional<CircleLineHit> collidesCircleLine(const Circle& c, const Line& l);
std::optional<CircleLineHit> collidesCircleCircle(const Circle& ball, const Circle& other);
} // namespace physics

} // namespace program