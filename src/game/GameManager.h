#pragma once

#include "client/Render.h"
#include "game/actions/AbstractAction.h"
#include "State.h"
#include <string>

namespace program {

class GameManager {
  State state;
  sdl2w::Window& window;
  Render r;
  std::string lastKeyPressed;
  double physicsAccumulator = 0.0;

public:
  GameManager(sdl2w::Window& windowA, GameParams gameParams = GameParams());
  ~GameManager();

  void load();
  void start();
  void handleKeyPress(const std::string& key);
  void handleKeyRelease(const std::string& key);
  void update(int dt);
  void render(int dt);
};
} // namespace program