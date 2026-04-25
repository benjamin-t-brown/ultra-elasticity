# ULTRA ELASTICITY

10 levels of the classic game of breakout with a few new mechanics.

The web version of this game is available here: https://benjamin-t-brown.github.io/ultra-elasticity/

<img width="640" height="480" alt="ss1" src="https://github.com/user-attachments/assets/26242683-66ca-4f60-be4f-964c5ea16cda" />
<img width="640" height="480" alt="ss2" src="https://github.com/user-attachments/assets/5328d790-a331-4a3a-96d5-1adb1ed9b930" />
<img width="640" height="480" alt="ss3" src="https://github.com/user-attachments/assets/9dae53c1-773a-4964-b2da-36f908aecdf2" />

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
