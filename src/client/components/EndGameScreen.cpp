#include "EndGameScreen.h"
#include "client/Render.h"
#include "client/UiRenderer.h"
#include "lib/sdl2w/Window.h"
#include <algorithm>
#include <sstream>

namespace program {

using sdl2w::FONT_DEFAULT;
using sdl2w::RenderTextParams;
using sdl2w::TextSize;

void EndGameScreen::setup(State& state) {
  localState = &state.uiInfo.endGameScreenState;
}

bool EndGameScreen::isSetup() const {
  //
  return localState != nullptr;
}

void EndGameScreen::reset(EndGameScreenState& s) {
  s.isBaseScoreTextVisible = false;
  s.isJuggleBonusTextVisible = false;
  s.isComboTextVisible = false;
  s.isScoreTextVisible = false;
  s.isTimeTextVisible = false;
  s.isRetriesTextVisible = false;
  s.isPressButtonTextVisible = false;
  s.isNewBestScoreTextVisible = false;
  s.isNewBestComboTextVisible = false;
  s.isNewBestTimeTextVisible = false;
  s.newBestText.reset();
  s.pressButtonText.reset();
}

void EndGameScreen::update(int dt) {
  if (!isSetup()) {
    return;
  }

  auto& s = *localState;
  if (s.isNewBestScoreTextVisible || s.isNewBestComboTextVisible ||
      s.isNewBestTimeTextVisible) {
    s.newBestText.update(dt);
  }
  if (s.isPressButtonTextVisible) {
    s.pressButtonText.update(dt);
  }
}

void EndGameScreen::renderPulsingText(
    const PulsingText& t, const sdl2w::RenderTextParams& textParams) {
  sdl2w::Draw& d = rPtr->getWindow().getDraw();
  d.drawText(t.text,
             sdl2w::RenderTextParams{
                 .fontName = textParams.fontName,
                 .fontSize = textParams.fontSize,
                 .x = textParams.x,
                 .y = textParams.y,
                 .color = textParams.color,
                 .centered = textParams.centered,
                 .angleDeg = textParams.angleDeg,
                 .scale = {t.getScale(), t.getScale()},
             });
}

void EndGameScreen::render() {
  if (!isSetup()) {
    return;
  }

  const auto& s = *localState;
  auto& state = rPtr->getState();
  auto [w, h] = rPtr->getWindow().getRenderDims();
  sdl2w::Draw& d = rPtr->getWindow().getDraw();
  d.drawText(s.titleText,
             RenderTextParams{
                 .fontName = FONT_DEFAULT,
                 .fontSize = sdl2w::TextSize::TEXT_SIZE_32,
                 .x = w / 2,
                 .y = h / 2 - 150,
                 .color = COLOR_WHITE,
                 .centered = true,
                 .angleDeg = -10.,
             });

  const int tableY = h / 2 - 70;
  const int rowGap = 32;
  const int labelX = w / 2 - 180 + 50;
  const int valueX = w / 2 + 20 + 50;
  const int newBestX = valueX + 110;
  const int stageScore = std::max(0, state.stageScore);
  const int stageStartScore = state.stageStartScore;
  const int timeSeconds = state.stopwatchMs / 1000;

  auto drawLabelValueRow = [&](int row,
                               const std::string& label,
                               const std::string& value,
                               const SDL_Color& valueColor = COLOR_WHITE,
                               const bool isLarge = false) {
    const int y = tableY + row * rowGap;
    d.drawText(label,
               sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                       .fontSize = isLarge
                                                       ? TextSize::TEXT_SIZE_24
                                                       : TextSize::TEXT_SIZE_20,
                                       .x = labelX,
                                       .y = isLarge ? y + 4 : y,
                                       .color = COLOR_WHITE,
                                       .centered = false});
    d.drawText(value,
               sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                       .fontSize = isLarge
                                                       ? TextSize::TEXT_SIZE_24
                                                       : TextSize::TEXT_SIZE_20,
                                       .x = valueX,
                                       .y = isLarge ? y + 4 : y,
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
  if (s.isComboTextVisible) {
    std::stringstream ss;
    ss << state.lastMaxCombo << " x 1000";
    drawLabelValueRow(3, s.comboText, ss.str());
    if (s.isNewBestComboTextVisible) {
      renderPulsingText(
          s.newBestText,
          sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                  .fontSize = TextSize::TEXT_SIZE_16,
                                  .x = newBestX,
                                  .y = tableY + 3 * rowGap + 6,
                                  .color = COLOR_YELLOW,
                                  .centered = true,
                                  .angleDeg = 10.});
    }
  }
  if (s.isTimeTextVisible) {
    std::stringstream ss;
    ss << timeSeconds << "s";
    drawLabelValueRow(4, s.timeText, ss.str());
    if (s.isNewBestTimeTextVisible) {
      renderPulsingText(
          s.newBestText,
          sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                  .fontSize = TextSize::TEXT_SIZE_16,
                                  .x = newBestX,
                                  .y = tableY + 4 * rowGap + 6,
                                  .color = COLOR_YELLOW,
                                  .centered = true,
                                  .angleDeg = 10.});
    }
  }
  int scoreTableRow = 5;
  if (s.isRetriesTextVisible) {
    drawLabelValueRow(5,
                      s.retriesText,
                      "-5000 x" + std::to_string(state.numContinuesUsed),
                      {255, 10, 10, 255});
    scoreTableRow = 6;
  }
  if (s.isScoreTextVisible) {
    drawLabelValueRow(scoreTableRow,
                      s.scoreText,
                      std::to_string(state.lastScore),
                      COLOR_WHITE,
                      true);
    if (s.isNewBestScoreTextVisible) {
      renderPulsingText(
          s.newBestText,
          sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                  .fontSize = TextSize::TEXT_SIZE_20,
                                  .x = newBestX,
                                  .y = tableY + scoreTableRow * rowGap + 15,
                                  .color = COLOR_YELLOW,
                                  .centered = true,
                                  .angleDeg = 10.});
    }
  }
  if (s.isPressButtonTextVisible && s.pressButtonText.isVisible) {
    d.drawText(
        s.pressButtonText.text,
        sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_24,
                                .x = w / 2,
                                .y = h / 2 + 180,
                                .color = COLOR_WHITE,
                                .centered = true});
  }
}
} // namespace program