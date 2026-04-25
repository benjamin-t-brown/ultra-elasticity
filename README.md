# ULTRA ELASTICITY

10 levels of the classic game of breakout with a few new mechanics.

The web version of this game is available here: https://benjamin-t-brown.github.io/ultra-elasticity/

## Quick Start

Development Requirements

- GCC ^15
- SDL2
- SDL2_image
- SDL2_ttf
- SDL2_mixer
- Node ^20

For Building WASM Executable

- Emscripten

The native program is built with the Makefile in the src directory.

```
cd src
make -j8
make run
```

For the web version, use npm:

```
npm run build
npm run dist
```

##

This project is a C++ game making use of SDL2 via the [sdl2w](https://github.com/benjamin-t-brown/sdl2w) wrapper lib.
