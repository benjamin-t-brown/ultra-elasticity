#pragma once

#include "components/EndGameScreen.h"
#include "components/InterLevelScreen.h"
#include <memory>

namespace program {
class Render;

#define COLOR_YELLOW {250, 206, 0, 255}
#define COLOR_WHITE {255, 255, 255, 255}
#define COLOR_CYAN {30, 235, 250, 255}

class Ui {
private:
  Render* rPtr;

  std::unique_ptr<EndGameScreen> endGameScreen;
  std::unique_ptr<InterLevelScreen> interLevelScreen;

public:
  Ui(Render& r);

  void setup();

  void update(int dt);

  void renderMenu();
  void renderPreGame();
  void renderInGame();
  void renderInterLevel();
  void renderDefeated();
  void renderGameOver();

  void render();
  void renderAboveFade();
};
} // namespace program