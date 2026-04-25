#include "InterLevelScreen.h"
#include "client/Render.h"
#include "client/UiRenderer.h"
#include "game/State.h"
#include "lib/sdl2w/Window.h"
#include <algorithm>
#include <sstream>

namespace program {

using sdl2w::FONT_DEFAULT;

void InterLevelScreen::setup(State& state) {
  localState = &state.uiInfo.interLevelScreenState;
}

bool InterLevelScreen::isSetup() const { return localState != nullptr; }

void InterLevelScreen::reset(InterLevelScreenState& s) {
  s.isBaseScoreTextVisible = false;
  s.isJuggleBonusTextVisible = false;
  s.isScoreTextVisible = false;
  s.isContinueTextVisible = false;
  s.continueText.reset();
}

void InterLevelScreen::update(int dt) {
  if (!isSetup()) {
    return;
  }
  auto& s = *localState;
  if (s.isContinueTextVisible) {
    s.continueText.update(dt);
  }
}

void InterLevelScreen::render() {
  if (!isSetup()) {
    return;
  }

  sdl2w::Draw& d = rPtr->getWindow().getDraw();
  const State& state = rPtr->getState();
  const auto& s = *localState;
  auto [w, h] = rPtr->getWindow().getRenderDims();

  d.drawText(s.titleText,
             sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                     .fontSize = sdl2w::TextSize::TEXT_SIZE_32,
                                     .x = w / 2,
                                     .y = h / 2 - 100,
                                     .color = COLOR_WHITE,
                                     .centered = true});

  const int tableY = h / 2 - 10;
  const int rowGap = 32;
  const int labelX = w / 2 - 180 + 65;
  const int valueX = w / 2 + 20 + 65;
  const int stageScore = std::max(0, state.stageScore);
  const int stageStartScore = std::max(0, state.stageStartScore);

  auto drawLabelValueRow = [&](int row,
                               const std::string& label,
                               const std::string& value,
                               const SDL_Color& valueColor = COLOR_WHITE) {
    const int y = tableY + row * rowGap;
    d.drawText(
        label,
        sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                                .x = labelX,
                                .y = y,
                                .color = COLOR_WHITE,
                                .centered = false});
    d.drawText(
        value,
        sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                                .x = valueX,
                                .y = y,
                                .color = valueColor,
                                .centered = false});
  };

  if (s.isBaseScoreTextVisible) {
    drawLabelValueRow(0,
                      s.stageStartScoreText,
                      std::to_string(stageStartScore),
                      COLOR_YELLOW);
    drawLabelValueRow(1, s.baseScoreText, std::to_string(stageScore));
  }
  if (s.isJuggleBonusTextVisible) {
    std::stringstream ss;
    ss << "x" << (state.interLevelJuggleTier + 1);
    drawLabelValueRow(2, s.juggleBonusText, ss.str(), COLOR_CYAN);
  }
  if (s.isScoreTextVisible) {
    drawLabelValueRow(3, s.scoreText, std::to_string(state.score), COLOR_WHITE);
  }

  if (s.isContinueTextVisible && s.continueText.isVisible) {
    d.drawText(
        s.continueText.text,
        sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_24,
                                .x = w / 2,
                                .y = h / 2 + 150,
                                .color = COLOR_WHITE,
                                .centered = true});
  }
}
} // namespace program
