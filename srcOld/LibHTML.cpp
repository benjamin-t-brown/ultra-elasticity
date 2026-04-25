#include "LibHTML.h"

#include "GameOptions.h"
#include "SDL2Wrapper.h"
#include <string>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

void notifyGameStarted() {
  const std::string script = std::string("window.Lib.notifyGameStarted()");
#ifdef __EMSCRIPTEN__
  emscripten_run_script(script.c_str());
#else
  LOG(INFO) << "notifyGameStarted: " << script << LOG_ENDL;
#endif
}
void notifyGameReady() {
  const std::string script = std::string("window.Lib.notifyGameReady()");
#ifdef __EMSCRIPTEN__
  emscripten_run_script(script.c_str());
#else
  LOG(INFO) << "notifyGameReady: " << script << LOG_ENDL;
#endif
}
void notifyGameCompleted(bool didWin, int score, int numRetries) {
  const std::string script = std::string(
      "window.Lib.notifyGameCompleted(" + std::to_string(didWin) + ", " +
      std::to_string(score) + ", " + std::to_string(numRetries) + ")");
#ifdef __EMSCRIPTEN__
  emscripten_run_script(script.c_str());
#else
  LOG(INFO) << "notifyGameCompleted: " << script << LOG_ENDL;
#endif
}
