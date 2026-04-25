#pragma once

#include "UiComponent.h"
#include "client/TextState.h"
#include "lib/sdl2w/L10n.h"

namespace sdl2w {
struct RenderTextParams;
}

namespace program {

class Render;
struct State;

struct EndGameScreenState {
  std::string titleText = TRANSLATE("Mission Accomplished!");
  std::string stageStartScoreText = TRANSLATE("Stage Start Score");
  bool isBaseScoreTextVisible = false;
  std::string baseScoreText = TRANSLATE("Stage Score");
  bool isComboTextVisible = false;
  std::string comboText = TRANSLATE("Max Combo");
  bool isJuggleBonusTextVisible = false;
  std::string juggleBonusText = TRANSLATE("Juggle Bonus");
  bool isScoreTextVisible = false;
  std::string scoreText = TRANSLATE("Total Score");
  bool isTimeTextVisible = false;
  std::string timeText = TRANSLATE("Time");
  std::string retriesText = TRANSLATE("Retries");
  bool isRetriesTextVisible = false;
  bool isPressButtonTextVisible = false;
  BlinkingText pressButtonText = BlinkingText{
      .text = TRANSLATE("Press button."),
  };
  bool isNewBestScoreTextVisible = false;
  bool isNewBestComboTextVisible = false;
  bool isNewBestTimeTextVisible = false;
  PulsingText newBestText = PulsingText{
    .text = TRANSLATE("New Best!"),
};
};

class EndGameScreen : UiComponent {
  EndGameScreenState* localState = nullptr;

public:
  EndGameScreen(Render& r) : UiComponent(r) {}
  static void reset(EndGameScreenState& state);
  void setup(State& state);
  bool isSetup() const override;
  void update(int dt) override;
  void renderPulsingText(const PulsingText& text,
                         const sdl2w::RenderTextParams& textParams);
  void render() override;
};

} // namespace program