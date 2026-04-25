#pragma once

namespace program {
class Render;
struct State;

class UiComponent {
protected:
  Render* rPtr;

public:
  UiComponent(Render& r) : rPtr(&r) {}
  virtual ~UiComponent() = default;
  virtual bool isSetup() const = 0;
  virtual void update(int dt) = 0;
  virtual void render() = 0;
};

} // namespace program