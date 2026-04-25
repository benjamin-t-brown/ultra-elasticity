#include "UiRenderer.h"
#include "Render.h"
#include "game/State.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/L10n.h"
#include "lib/sdl2w/Window.h"
#include <cmath>

namespace program {

using sdl2w::FONT_DEFAULT;

using sdl2w::RenderableParams;

Ui::Ui(Render& r) : rPtr(&r) {
  // preload these
  rPtr->getAnim("wall_top_in");
  rPtr->getAnim("wall_top_out");
  rPtr->getAnim("wall_bottom_in");
  rPtr->getAnim("wall_bottom_out");
  rPtr->getAnim("wall_left_in");
  rPtr->getAnim("wall_left_out");
  rPtr->getAnim("wall_right_in");
  rPtr->getAnim("wall_right_out");
}

void Ui::setup() {
  endGameScreen = std::make_unique<EndGameScreen>(*rPtr);
  endGameScreen->setup(rPtr->getStateMutable());
  interLevelScreen = std::make_unique<InterLevelScreen>(*rPtr);
  interLevelScreen->setup(rPtr->getStateMutable());
}

void Ui::update(int dt) {
  if (endGameScreen) {
    endGameScreen->update(dt);
  }
  if (interLevelScreen) {
    interLevelScreen->update(dt);
  }
}

void Ui::renderMenu() {
  sdl2w::Draw& d = rPtr->getWindow().getDraw();
  const State& state = rPtr->getState();
  auto [w, h] = rPtr->getWindow().getRenderDims();

  auto textStartX = w / 2;
  auto textStartY = h / 2 - 100;

  d.drawText(TRANSLATE("ELASTICITY"),
             sdl2w::RenderTextParams{
                 .fontName = FONT_DEFAULT,
                 .fontSize = sdl2w::TextSize::TEXT_SIZE_60,
                 .x = textStartX - 2,
                 .y = textStartY + 2,
                 .color = {150, 150, 150, 255},
                 .centered = true,
             });
  d.drawText(TRANSLATE("ELASTICITY"),
             sdl2w::RenderTextParams{
                 .fontName = FONT_DEFAULT,
                 .fontSize = sdl2w::TextSize::TEXT_SIZE_60,
                 .x = textStartX,
                 .y = textStartY,
                 .color = {255, 255, 255, 255},
                 .centered = true,
             });
  d.drawText(TRANSLATE("ULTRA"),
             sdl2w::RenderTextParams{
                 .fontName = FONT_DEFAULT,
                 .fontSize = sdl2w::TextSize::TEXT_SIZE_36,
                 .x = textStartX,
                 .y = textStartY - 50,
                 .color = COLOR_YELLOW,
                 .centered = true,
                 .angleDeg = -10.,
                 .scale = {state.uiInfo.ultraScale, state.uiInfo.ultraScale},
             });

  if (state.bestTime > 0) {
    d.drawText(
        TRANSLATE("BEST COMBO: ") + std::to_string(state.bestMaxCombo),
        sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                                .x = w / 2,
                                .y = h / 2 + 10,
                                .color = COLOR_WHITE,
                                .centered = true});

    d.drawText(
        TRANSLATE("BEST TIME: ") + std::to_string(state.bestTime) + "s",
        sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                                .x = w / 2,
                                .y = h / 2 + 30,
                                .color = COLOR_YELLOW,
                                .centered = true});

    d.drawText(
        TRANSLATE("BEST SCORE: ") + std::to_string(state.bestScore),
        sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                                .x = w / 2,
                                .y = h / 2 + 50,
                                .color = COLOR_CYAN,
                                .centered = true});
  }

  if (state.uiInfo.startGameText.isVisible) {
    d.drawText(state.uiInfo.startGameText.text,
               sdl2w::RenderTextParams{
                   .fontName = FONT_DEFAULT,
                   .fontSize = sdl2w::TextSize::TEXT_SIZE_24,
                   .x = textStartX,
                   .y = h / 2 + 150,
                   .color = {210, 210, 210, 255},
                   .centered = true,
               });
  }
}

void Ui::renderPreGame() {
  // sdl2w::Draw& d = rPtr->getWindow().getDraw();
  // d.drawRect(0, 0, 100, 100, {0, 100, 0, 255});
}

void Ui::renderInGame() {
  sdl2w::Draw& d = rPtr->getWindow().getDraw();
  const State& state = rPtr->getState();
  auto [w, h] = rPtr->getWindow().getRenderDims();

  auto& animTopIn = rPtr->getAnim("wall_top_in");
  auto& animTopOut = rPtr->getAnim("wall_top_out");
  auto& animBottomIn = rPtr->getAnim("wall_bottom_in");
  auto& animBottomOut = rPtr->getAnim("wall_bottom_out");
  auto& animLeftIn = rPtr->getAnim("wall_left_in");
  auto& animLeftOut = rPtr->getAnim("wall_left_out");
  auto& animRightIn = rPtr->getAnim("wall_right_in");
  auto& animRightOut = rPtr->getAnim("wall_right_out");
  auto [renderW, renderH] = rPtr->getWindow().getRenderDims();
  const int wallHeight = 60;
  const int wallWidth = 80;

  if (state.levelInfo.areProtectionWallsEnabled) {
    for (int i = 0; i < state.levelInfo.playAreaWidth; i += wallWidth) {
      d.drawAnimation(
          animTopIn,
          sdl2w::RenderableParams{
              .scale = {1., 1.}, .x = i, .y = 0, .centered = false});
      d.drawAnimation(animBottomIn,
                      sdl2w::RenderableParams{.scale = {1., 1.},
                                              .x = i,
                                              .y = renderH - wallHeight,
                                              .centered = false});
    }
    for (int i = 0; i < state.levelInfo.playAreaHeight; i += wallHeight) {
      d.drawAnimation(
          animLeftIn,
          sdl2w::RenderableParams{
              .scale = {1., 1.}, .x = 0, .y = i, .centered = false});
      d.drawAnimation(animRightIn,
                      sdl2w::RenderableParams{.scale = {1., 1.},
                                              .x = renderW - wallWidth,
                                              .y = i,
                                              .centered = false});
    }
  } else {
    for (int i = 0; i < state.levelInfo.playAreaWidth; i += wallWidth) {
      d.drawAnimation(
          animTopOut,
          sdl2w::RenderableParams{
              .scale = {1., 1.}, .x = i, .y = 0, .centered = false});
      d.drawAnimation(animBottomOut,
                      sdl2w::RenderableParams{.scale = {1., 1.},
                                              .x = i,
                                              .y = renderH - wallHeight,
                                              .centered = false});
    }
    for (int i = 0; i < state.levelInfo.playAreaHeight; i += wallHeight) {
      d.drawAnimation(
          animLeftOut,
          sdl2w::RenderableParams{
              .scale = {1., 1.}, .x = 0, .y = i, .centered = false});
      d.drawAnimation(animRightOut,
                      sdl2w::RenderableParams{.scale = {1., 1.},
                                              .x = renderW - wallWidth,
                                              .y = i,
                                              .centered = false});
    }

    std::stringstream ss;
    ss << std::string(TRANSLATE("Max Combo: ")) << state.maxCombo;
    d.drawText(
        ss.str(),
        sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                                .x = 16,
                                .y = 4,
                                .color = COLOR_YELLOW,
                                .centered = false});
    d.drawText(
        std::string(TRANSLATE("Combo: ")) + std::to_string(state.combo),
        sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_16,
                                .x = 16,
                                .y = 24,
                                .color = COLOR_WHITE,
                                .centered = false});
    d.drawText(
        std::string(TRANSLATE("Score: ")) +
            std::to_string(state.stageStartScore + state.stageScore),
        sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                                .x = w / 2,
                                .y = 16,
                                .color = COLOR_CYAN,
                                .centered = true});
    const int totalSeconds = state.stopwatchMs / 1000;
    const int minutes = totalSeconds / 60;
    const int seconds = totalSeconds % 60;
    std::stringstream stopwatchSs;
    stopwatchSs << (minutes < 10 ? "0" : "") << minutes << ":"
                << (seconds < 10 ? "0" : "") << seconds;
    const std::string stopwatchLabel = stopwatchSs.str();
    const auto stopwatchSize =
        d.measureText(stopwatchLabel,
                      sdl2w::RenderTextParams{
                          .fontName = FONT_DEFAULT,
                          .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                      });
    d.drawText(stopwatchLabel,
               sdl2w::RenderTextParams{
                   .fontName = FONT_DEFAULT,
                   .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                   .x = w - stopwatchSize.first - 16,
                   .y = 4,
                   .color = COLOR_WHITE,
                   .centered = false,
               });

    const std::string levelLabel =
        TRANSLATE("Level ") + std::to_string(state.levelInfo.levelIndex + 1);
    d.drawText(levelLabel,
               sdl2w::RenderTextParams{
                   .fontName = FONT_DEFAULT,
                   .fontSize = sdl2w::TextSize::TEXT_SIZE_16,
                   .x = w - 77,
                   .y = h - 22,
                   .color = COLOR_WHITE,
                   .centered = false,
               });

    if (state.uiInfo.launchBallText.isVisible &&
        state.controlState == CONTROL_IN_GAME &&
        state.playerPaddle.numBalls > 0) {
      d.drawText(
          state.uiInfo.launchBallText.text,
          sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                  .fontSize = sdl2w::TextSize::TEXT_SIZE_16,
                                  .x = w / 2,
                                  .y = h - 12,
                                  .color = COLOR_WHITE,
                                  .centered = true});
    }
  }

  // Juggle gauge: linear tiers (equal spacing), fill scales with active balls.
  {
    const int barW = 4;
    const int barH = h / 2 - 100;
    const int barX = w - 25;
    const int barY = h / 2 + 25;
    const int maxBallsSeen = std::max(1, state.juggleGaugeMaxBallsSeen);
    const double effectivePips = maxBallsSeen;
    double fillPct = state.juggleGauge.value / effectivePips;
    if (fillPct < 0.0) {
      fillPct = 0.0;
    } else if (fillPct > 1.0) {
      fillPct = 1.0;
    }
    const int fillH = static_cast<int>(fillPct * barH);

    d.drawRect(barX - 2, barY - 2, barW + 4, barH + 4, {30, 30, 30, 220});
    d.drawRect(barX, barY, barW, barH, {70, 70, 70, 220});
    if (fillH > 0) {
      d.drawRect(barX, barY + barH - fillH, barW, fillH, COLOR_YELLOW);
    }

    // Draw threshold notches for each ball-count cap.
    for (int ballCount = 1; ballCount <= maxBallsSeen; ballCount++) {
      const int notchY = barY + barH -
                         static_cast<int>((static_cast<double>(ballCount) /
                                           static_cast<double>(maxBallsSeen)) *
                                          barH);
      const int prevNotchY =
          barY + barH -
          static_cast<int>((static_cast<double>(ballCount - 1) /
                            static_cast<double>(maxBallsSeen)) *
                           barH);
      auto color = (ballCount <= state.juggleGaugeCompletedTiers)
                       ? SDL_Color(COLOR_YELLOW)
                       : SDL_Color(COLOR_CYAN);
      d.drawRect(barX - 5, notchY - 1, barW + 10, 2, color);
      d.drawText(
          "x" + std::to_string(ballCount + 1),
          sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                  .fontSize = sdl2w::TextSize::TEXT_SIZE_12,
                                  .x = barX - 18,
                                  .y = notchY - 10,
                                  .color = COLOR_WHITE,
                                  .centered = false});
      if (ballCount > getActiveBallsInPlay(state) &&
          ballCount > state.juggleGaugeCompletedTiers &&
          state.controlState == CONTROL_IN_GAME && ballCount > 1) {
        d.drawRect(barX,
                   notchY + 1,
                   barW,
                   -notchY + prevNotchY - 2,
                   {255, 70, 70, 255});
      }
    }

    d.drawText(
        std::string(TRANSLATE("Juggle")),
        sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_12,
                                .x = w - 40,
                                .y = h - 65,
                                .color = COLOR_YELLOW,
                                .centered = false});
    d.drawText(
        std::string(TRANSLATE("Bonus")),
        sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_12,
                                .x = w - 38,
                                .y = h - 50,
                                .color = COLOR_YELLOW,
                                .centered = false});
  }

  // Protection walls timer progress bar
  if (state.levelInfo.areProtectionWallsEnabled) {
    const double remaining =
        1.0 - timer::getPct(state.levelInfo.protectionWallsTimer);
    const int barW = 160;
    const int barH = 8;
    const int barX = w / 2 - barW / 2;
    const int barY = 36;
    d.drawRect(barX, barY, barW, barH, {40, 40, 80, 200});
    d.drawRect(
        barX, barY, static_cast<int>(barW * remaining), barH, COLOR_CYAN);
  }

  // Short paddle timer progress bar
  if (state.playerPaddle.paddleState == PLAYER_PADDLE_SHORT) {
    double remaining = 1.0 - timer::getPct(state.playerPaddle.shortPaddleTimer);
    if (remaining < 0.0) {
      remaining = 0.0;
    } else if (remaining > 1.0) {
      remaining = 1.0;
    }
    const int barW = 40;
    const int barH = 2;
    const int barX = static_cast<int>(state.playerPaddle.physics.x) - barW / 2;
    const int barY = static_cast<int>(state.playerPaddle.physics.y) + 24;
    d.drawRect(barX, barY, barW, barH, {255, 255, 255, 35});
    d.drawRect(barX,
               barY,
               static_cast<int>(barW * remaining),
               barH,
               {255, 255, 255, 120});
  }

  if (state.playerPaddle.numBalls > 0) {
    d.drawSprite(
        rPtr->getWindow().getStore().getSprite("ball_stored_0"),
        sdl2w::RenderableParams{
            .scale = {1., 1.}, .x = 16, .y = h - 12, .centered = true});
    std::stringstream ballMessage;
    ballMessage << state.playerPaddle.numBalls;
    d.drawText(
        ballMessage.str(),
        sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_16,
                                .x = 42,
                                .y = h - 22,
                                .color = COLOR_WHITE,
                                .centered = false});
  }
}

void Ui::renderInterLevel() {
  if (interLevelScreen) {
    interLevelScreen->render();
  }
}

void Ui::renderDefeated() {
  sdl2w::Draw& d = rPtr->getWindow().getDraw();
  const State& state = rPtr->getState();
  auto [w, h] = rPtr->getWindow().getRenderDims();

  d.drawRect(0, 0, w, h, {0, 0, 0, 255});

  d.drawText(TRANSLATE("DEFEATED"),
             sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                     .fontSize = sdl2w::TextSize::TEXT_SIZE_48,
                                     .x = w / 2,
                                     .y = h / 2 - 120,
                                     .color = {220, 80, 80, 255},
                                     .centered = true});

  if (state.uiInfo.defeatedRestartPrompt.isVisible) {
    d.drawText(state.uiInfo.defeatedRestartPrompt.text,
               sdl2w::RenderTextParams{
                   .fontName = FONT_DEFAULT,
                   .fontSize = sdl2w::TextSize::TEXT_SIZE_24,
                   .x = w / 2,
                   .y = h / 2 - 20,
                   .color = COLOR_WHITE,
                   .centered = true,
               });
  }

  const auto& t = state.uiInfo.defeatedAutoMenuTimer;
  const double remainingMs = std::max(0.0, t.duration - t.t);
  const int secondsLeft = static_cast<int>(std::ceil(remainingMs / 1000.0));
  if (secondsLeft > 0) {
    const std::string countdown =
        std::string(TRANSLATE("Menu in ")) + std::to_string(secondsLeft) + "s";
    d.drawText(
        countdown,
        sdl2w::RenderTextParams{.fontName = FONT_DEFAULT,
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                                .x = w / 2,
                                .y = h / 2 + 100,
                                .color = {200, 200, 200, 255},
                                .centered = true});
  }
}

void Ui::renderGameOver() {
  //
  endGameScreen->render();
}

void Ui::render() {
  if (!rPtr->isSetup()) {
    return;
  }
  const State& state = rPtr->getState();

  switch (state.uiState) {
  case UI_HIDDEN:
    return;
  case UI_MENU:
    renderMenu();
    break;
  case UI_PRE_GAME:
    renderPreGame();
    break;
  case UI_IN_GAME:
    renderInGame();
    break;
  case UI_DEFEATED:
    renderDefeated();
    break;
  default:
    break;
  }
}

void Ui::renderAboveFade() {
  if (!rPtr->isSetup()) {
    return;
  }
  const State& state = rPtr->getState();
  switch (state.uiState) {
  case UI_INTER_LEVEL:
    renderInterLevel();
    break;
  case UI_GAME_OVER:
    renderGameOver();
    break;
  default:
    break;
  }
}

} // namespace program