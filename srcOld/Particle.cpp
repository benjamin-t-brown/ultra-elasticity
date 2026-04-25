#include "Particle.h"
#include "Game.h"
#include "GameOptions.h"
#include "Player.h"
#include <sstream>

Particle::Particle(Game& gameA, const std::string& particleTypeA, int ms)
    : Actor(gameA, "invisible"),
      particleType("text"),
      timer(addFuncTimer(ms, [&]() { remove(); })),
      text("") {
  setParticleType(particleTypeA);
}

Particle::~Particle() {}

void Particle::setParticleType(const std::string& particleTypeA) {
  particleType = particleTypeA;
  if (particleType == "fade_in") {
    setV(0, 0);
    set(0, 0);
  } else if (particleType == "fade_out") {
    setV(0, 0);
    set(0, 0);
  } else if (particleType == "black") {
    setV(0, 0);
    set(0, 0);
  } else if (particleType == "text") {
    setV(0, -1.0);
  }
}

void Particle::update() {
  Actor::update();
  if (y > GameOptions::height) {
    y = GameOptions::height;
    remove();
  }
}

void Particle::draw() {
  sdl2w::Draw& d = game.window.getDraw();
  if (particleType == "text") {
    d.drawText(text,
               sdl2w::RenderTextParams{.fontName = "default",
                                       .fontSize = sdl2w::TextSize::TEXT_SIZE_18,
                                       .x = static_cast<int>(x),
                                       .y = static_cast<int>(y),
                                       .color = {255, 255, 255, 255},
                                       .centered = true});
  } else if (particleType == "fade_out") {
    d.setGlobalAlpha(static_cast<int>(timer.getPctComplete() * 255));
    d.drawSprite(game.window.getStore().getSprite("cpp_splash_black"),
                 sdl2w::RenderableParams{.x = 0, .y = 0});
    d.setGlobalAlpha(255);
  } else if (particleType == "fade_in") {
    d.setGlobalAlpha(static_cast<int>((1 - timer.getPctComplete()) * 255));
    d.drawSprite(game.window.getStore().getSprite("cpp_splash_black"),
                 sdl2w::RenderableParams{.x = 0, .y = 0});
    d.setGlobalAlpha(255);
  } else if (particleType == "black") {
    d.drawSprite(game.window.getStore().getSprite("cpp_splash_black"),
                 sdl2w::RenderableParams{.x = 0, .y = 0});
  }
}
