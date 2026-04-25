#pragma once

#include "SDL2Wrapper.h"

#include <unordered_map>

class Game;

class Actor {
protected:
  Game& game;
  std::vector<std::unique_ptr<Timer>> timers;
  bool removeFlag;

public:
  std::string animState;
  std::string spriteBase;
  double x;
  double y;
  double vx;
  double vy;
  double ax;
  double ay;
  float r;
  std::unordered_map<std::string, sdl2w::Animation> anims;
  Actor(Game& gameA, const std::string& spriteBaseA);
  virtual ~Actor();
  void set(const double xA, const double yA);
  void setV(const double vxA, const double vyA);
  void setA(const double axA, const double vyA);
  void setVx(const double vxA);
  void setVy(const double vyA);
  void setAx(const double axA);
  void setAy(const double ayA);
  void setAnimState(const std::string& state);
  Timer& addBoolTimer(const int ms, bool& ref);
  Timer& addFuncTimer(const int ms, std::function<void()> cb);
  void remove();
  bool shouldRemove() const;

  virtual void update();
  virtual void draw();
};
