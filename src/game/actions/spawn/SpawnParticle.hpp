

#pragma once

#include "game/State.h"
#include "game/actions/AbstractAction.h"
#include "lib/sdl2w/Animation.h"

namespace program {

namespace actions {

class SpawnParticle : public AbstractAction {
  std::string animName;
  double x;
  double y;
  int durationMs;
  bool isText;
  std::string text;
  sdl2w::RenderTextParams textParams;

  void act() override {
    State& localState = *this->state;
    if (localState.store == nullptr) {
      return;
    }
    if (isText) {
      localState.particles.push_back(std::make_unique<Particle>(Particle{
          .timer = Timer{static_cast<double>(durationMs), 0},
          .animName = animName,
          .text = text,
          .textParams = textParams,
          .x = static_cast<int>(x),
          .y = static_cast<int>(y),
          .ms = durationMs,
      }));
    } else {
      sdl2w::AnimationDefinition def =
          localState.store->getAnimationDefinition(animName);
      auto anim = std::unique_ptr<sdl2w::Animation>(
          new sdl2w::Animation(def, *localState.store));
      localState.particles.push_back(std::make_unique<Particle>(Particle{
          .animation = std::move(anim),
          .timer = Timer{static_cast<double>(durationMs), 0},
          .animName = animName,
          .x = static_cast<int>(x),
          .y = static_cast<int>(y),
          .ms = durationMs,
      }));
    }
  }

public:
  SpawnParticle(const std::string& animName, double x, double y, int durationMs)
      : animName(animName), x(x), y(y), durationMs(durationMs), isText(false) {}
  SpawnParticle(const std ::string& text,
                double x,
                double y,
                int durationMs,
                const sdl2w::RenderTextParams& textParams)
      : animName(text),
        x(x),
        y(y),
        durationMs(durationMs),
        isText(true),
        text(text),
        textParams(textParams) {}
};

} // namespace actions

} // namespace program
