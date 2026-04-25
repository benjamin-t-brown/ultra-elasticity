#include "Game.h"
#include "GameOptions.h"
#include "SDL2Wrapper.h"
#include <algorithm>
#include <ctime>
#include <functional>
#include <iostream>
#include <map>
#include <vector>

#include "LibHTML.h"

const std::string GameOptions::programName = "Ultra Elasticity";

void parseArgs(int argc, char* argv[], std::vector<std::string>& args) {
  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];
    if (arg.size() > 2 && arg.at(0) == '-' && arg.at(1) == '-') {
      arg = arg.substr(2);
      args.push_back(arg);
    }
  }
}

bool includes(const std::string& arg, const std::vector<std::string>& args) {
  return std::find(args.begin(), args.end(), arg) != args.end();
}

int main(int argc, char* argv[]) {
  LOG(INFO) << "Program Begin." << LOG_ENDL;
  srand(time(NULL));

  std::vector<std::string> args;
  parseArgs(argc, argv, args);

  try {
    sdl2w::Window::init();

    sdl2w::Store store;
    sdl2w::Window window(store,
                         {
                             .title = GameOptions::programName,
                             .w = GameOptions::width,
                             .h = GameOptions::height,
                             .x=200,
                             .y=200,
                             .renderW = GameOptions::width,
                             .renderH = GameOptions::height,
                         });

    window.getStore().loadAndStoreFont("default", "assets/monofonto.ttf");

    sdl2w::AssetLoader assetLoader(window.getDraw(), window.getStore());
    assetLoader.loadAssetsFromFile(sdl2w::DEPRECATED_ASSET_TYPE_SPRITE,
                                   "assets/sprites.txt");
    assetLoader.loadAssetsFromFile(sdl2w::DEPRECATED_ASSET_TYPE_SPRITE,
                                   "assets/intro_sprites.txt");
    assetLoader.loadAssetsFromFile(sdl2w::DEPRECATED_ASSET_TYPE_ANIMATION,
                                   "assets/anims.txt");
    assetLoader.loadAssetsFromFile(sdl2w::DEPRECATED_ASSET_TYPE_SOUND,
                                   "assets/sounds.txt");
    assetLoader.loadAssetsFromFile(sdl2w::DEPRECATED_ASSET_TYPE_SOUND,
                                   "assets/intro_sounds.txt");

    Game game(window);

    bool isWaitingToStart = includes("wait", args);

    auto pressButton = [&](const std::string& /*key*/, int) {
      isWaitingToStart = false;
    };
    sdl2w::Events& events = window.getEvents();
    events.setKeyboardEvent(sdl2w::ON_KEY_DOWN, pressButton);
    notifyGameReady();

    auto _initializeLoop = [&]() {
      sdl2w::renderSplash(window);
      return true;
    };

    auto _onInitialized = [&]() {
      game.enableMenu();
    };

    auto _mainLoop = [&]() {
      if (isWaitingToStart) {
        sdl2w::Draw& d = window.getDraw();
        d.drawText("Press button.",
                   sdl2w::RenderTextParams{.fontName = "default",
                                          .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                                          .x = GameOptions::width / 2,
                                          .y = GameOptions::height / 2,
                                          .color = {255, 255, 255, 255},
                                          .centered = true});
        return true;
      } else {
        return game.loop();
      }
    };

    window.startRenderLoop(_initializeLoop, _onInitialized, _mainLoop);

    sdl2w::Window::unInit();
    LOG(INFO) << "Program End." << LOG_ENDL;
  } catch (const std::string& e) {
    std::cout << e;
  }
  return 0;
}
