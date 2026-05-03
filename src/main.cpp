#include "game/GameManager.h"
#include "lib/sdl2w/AssetLoader.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/EmscriptenHelpers.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Init.h"
#include "lib/sdl2w/L10n.h"
#include "lib/sdl2w/Logger.h"

void runProgram(int argc, char** argv) {
  const int w = 640;
  const int h = 480;

  sdl2w::L10n::init(std::vector<std::string>({"en", "la"}));
  sdl2w::L10n::setEnabled(false); // FOR DEBUG

  sdl2w::Store store;
  sdl2w::Window window(store,
                       {
                           .mode = sdl2w::DrawMode::GPU,
                           .title = TRANSLATE("Ultra Elasticity"),
                           .w = !emshelpers::isEmscriptenEnv() ? w * 1 : w,
                           .h = !emshelpers::isEmscriptenEnv() ? h * 1 : h,
                           .x = 25, // SDL_WINDOWPOS_UNDEFINED
                           .y = 50, // SDL_WINDOWPOS_UNDEFINED
                           .renderW = w,
                           .renderH = h,
                       });

  sdl2w::setupStartupArgs(argc, argv, window);
  window.getDraw().setBackgroundColor({0, 0, 0});
  window.getStore().loadAndStoreFont("default", "assets/monofonto.ttf");

  sdl2w::AssetLoader assetLoader(window.getDraw(), window.getStore());
  assetLoader.loadAssetsFromFile(sdl2w::DEPRECATED_ASSET_TYPE_SPRITE,
                                 "assets/sprites.txt");
  assetLoader.loadAssetsFromFile(sdl2w::DEPRECATED_ASSET_TYPE_ANIMATION,
                                 "assets/anims.txt");
  assetLoader.loadAssetsFromFile(sdl2w::DEPRECATED_ASSET_TYPE_SOUND,
                                 "assets/sounds.txt");

  sdl2w::Draw& d = window.getDraw();
  program::GameManager game(window);
  game.load();
  window.setSoundPct(33);

  auto& events = window.getEvents();
  events.setKeyboardEvent(sdl2w::ON_KEY_DOWN, [&](const std::string& key, int) {
    game.handleKeyPress(key);
  });
  events.setKeyboardEvent(sdl2w::ON_KEY_UP, [&](const std::string& key, int) {
    game.handleKeyRelease(key);
  });

  auto _initializeLoop = [&]() {
    sdl2w::renderSplash(window);
    return true;
  };

  auto _onInitialized = [&]() { game.start(); };

  auto _mainLoop = [&]() {
    d.setBackgroundColor({10, 10, 10});

    if (!emshelpers::isEmscriptenEnv()) {
      if (window.getEvents().isKeyPressed("Escape")) {
        return false;
      }
    }
    auto dt = std::min(window.getDeltaTime(), 100);
    game.update(dt);
    game.render(dt);
    return true;
  };

  window.setInitTimeMax(5000); // show splash screen for at least 0.5 seconds
  window.startRenderLoop(_initializeLoop, _onInitialized, _mainLoop);
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start program" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  runProgram(argc, argv);

  sdl2w::Window::unInit();
  LOG(INFO) << "End program" << LOG_ENDL;
}
