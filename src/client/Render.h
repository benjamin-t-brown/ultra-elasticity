#pragma once

#include "game/State.h"
#include "lib/sdl2w/Animation.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace sdl2w {
class Window;
}

namespace program {
class Ui;

class Render {
  State* statePtr = nullptr;
  sdl2w::Window& window;
  std::unordered_map<std::string, std::unique_ptr<sdl2w::Animation>> animations;
  std::unique_ptr<program::Ui> ui;

public:
  Render(sdl2w::Window& windowA);
  void setup(State& state);
  bool isSetup() const { return statePtr != nullptr; }
  const State& getState() const { return *statePtr; }
  State& getStateMutable() { return *statePtr; }
  sdl2w::Window& getWindow() { return window; }

  sdl2w::Animation& getAnim(const std::string& name);
  sdl2w::Animation&
  getAnim(std::unordered_map<std::string, std::unique_ptr<sdl2w::Animation>>&
              animations,
          const std::string& name);

  int getSpriteOffsetLevel(int offset = 0);

  void update(int dt);
  void renderBackground();
  void renderBalls();
  void renderParticles();
  void renderBricks();
  void renderVentFans();
  void renderPlayerPaddle();
  void renderPowerups();
  void renderFadeBlackScreen();
  void renderAll();
};
} // namespace program