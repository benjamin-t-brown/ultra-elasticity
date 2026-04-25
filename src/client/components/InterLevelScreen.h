#pragma once

#include "UiComponent.h"
#include "client/TextState.h"
#include "lib/sdl2w/L10n.h"

namespace program {

class Render;
struct State;

struct InterLevelScreenState {
  std::string titleText = TRANSLATE("Stage Completed!");
  std::string stageStartScoreText = TRANSLATE("Stage Start Score");
  bool isBaseScoreTextVisible = false;
  std::string baseScoreText = TRANSLATE("Stage Score");
  bool isJuggleBonusTextVisible = false;
  std::string juggleBonusText = TRANSLATE("Juggle Bonus");
  bool isScoreTextVisible = false;
  std::string scoreText = TRANSLATE("Total Score");
  bool isContinueTextVisible = false;
  BlinkingText continueText = BlinkingText{
      .text = TRANSLATE("Press button to continue."),
  };
};

class InterLevelScreen : UiComponent {
  InterLevelScreenState* localState = nullptr;

public:
  InterLevelScreen(Render& r) : UiComponent(r) {}
  static void reset(InterLevelScreenState& state);
  void setup(State& state);
  bool isSetup() const override;
  void update(int dt) override;
  void render() override;
};

} // namespace program
