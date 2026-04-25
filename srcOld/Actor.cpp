#include "Actor.h"
#include "Game.h"

Actor::Actor(Game& gameA, const std::string& spriteBaseA)
    : game(gameA),
      removeFlag(false),
      animState("default"),
      spriteBase(spriteBaseA),
      x(0.0),
      y(0.0),
      vx(0.0),
      vy(0.0),
      ax(0.0),
      ay(0.0),
      r(16.0) {
  anims["default"] = game.window.getStore().createAnimation(spriteBaseA);
  setAnimState("default");
}

Actor::~Actor() { anims.clear(); }

void Actor::set(const double xA, const double yA) {
  x = xA;
  y = yA;
}
void Actor::setV(const double vxA, const double vyA) {
  vx = vxA;
  vy = vyA;
}
void Actor::setA(const double axA, const double ayA) {
  ax = axA;
  ay = ayA;
}
void Actor::setVx(const double vxA) { vx = vxA; }
void Actor::setVy(const double vyA) { vy = vyA; }
void Actor::setAx(const double axA) { ax = axA; }
void Actor::setAy(const double ayA) { ay = ayA; }

void Actor::setAnimState(const std::string& state) {
  if (animState != state) {
    if (anims.find(state) != anims.end()) {
      animState = state;
      sdl2w::Animation& anim = anims[animState];
      anim.start();
    }
  }
}

void Actor::remove() { removeFlag = true; }

bool Actor::shouldRemove() const { return removeFlag; }

Timer& Actor::addBoolTimer(const int ms, bool& ref) {
  timers.push_back(std::make_unique<BoolTimer>(ms, ref));
  return *timers.back();
}

Timer& Actor::addFuncTimer(const int ms, std::function<void()> cb) {
  timers.push_back(std::make_unique<FuncTimer>(ms, cb));
  return *timers.back();
}

void Actor::update() {
  int dt = game.window.getDeltaTime();
  double frameRatio = dt / (1000.0 / 60.0);

  vx += ax * frameRatio * frameRatio;
  vy += ay * frameRatio * frameRatio;

  x += vx * frameRatio;
  y += vy * frameRatio;

  unsigned int len = timers.size();
  for (unsigned int i = 0; i < len; i++) {
    Timer& timer = *timers[i];
    if (timer.shouldRemove()) {
      timers.erase(timers.begin() + i);
      i--;
      len--;
    }
  }
  for (unsigned int i = 0; i < len; i++) {
    timers[i]->update(dt);
  }
}

void Actor::draw() {
  sdl2w::Animation& anim = anims[animState];
  anim.update(game.window.getDeltaTime());
  sdl2w::Draw& d = game.window.getDraw();
  d.drawAnimation(anim, sdl2w::RenderableParams{.x = static_cast<int>(x),
                                                .y = static_cast<int>(y)});
}
