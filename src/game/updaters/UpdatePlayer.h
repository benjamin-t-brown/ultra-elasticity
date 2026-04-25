#pragma once

#include "game/State.h"

namespace sdl2w {
class Events;
}

namespace program {
void updatePlayer(State& state, sdl2w::Events& events, int dt);
} // namespace program