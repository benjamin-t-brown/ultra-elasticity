# Development

Here are some notes for development of this project.  In general, you should iterate with the native build, then run the web build when you are ready to export.  Everything here assumes you have a linux-like environment, be that actual Linux, a Mac, or MSYS2 on Windows.

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

### Ubuntu/Debian

```
apt install\
 build-essential\
 make\
 clangd-17\
 clang-format\
 libsdl2-ttf-dev\
 libsdl2-image-dev\
 libsdl2-mixer-dev\
 libsdl2-gfx-dev -y
```

### Mac M^ (brew)

```
brew install gcc@13
brew install brew install sdl2 sdl2_image sdl2_mixer sdl2_ttf

# additional development tools
python3 -m pip install --upgrade setuptools
python3 -m pip install --upgrade pip
```

### UCRT64/Mingw64 via MSYS2 (Windows)

Assuming you're using ucrt64 (other distribs like mingw64 or clang should work fine):

```
pacman -S ucrt-w64-x86_64-toolchain
pacman -S ucrt64/mingw-w64-ucrt-x86_64-SDL2\
 ucrt64/mingw-w64-ucrt-x86_64-SDL2_gfx\
 ucrt64/mingw-w64-ucrt-x86_64-SDL2_image\
 ucrt64/mingw-w64-ucrt-x86_64-SDL2_mixer\
 ucrt64/mingw-w64-ucrt-x86_64-SDL2_ttf

pacman -S ucrt64/mingw-w64-ucrt-x86_64-clang\
 ucrt64/mingw-w64-ucrt-x86_64-clang-tools-extra\
 ucrt64/mingw-w64-ucrt-x86_64-include-what-you-use
```

### VS Code / Cursor debugger path (native builds)

`launch.json` reads debugger settings from user settings so machine-specific paths are not committed to the repository.

Open User Settings JSON and set:

```json
{
  "cppdbg.miDebuggerPath": "C:\\progs\\msys2\\ucrt64\\bin\\gdb.exe",
  "cppdbg.msys2BinPath": "C:\\progs\\msys2\\ucrt64\\bin"
}
```

In Cursor/VS Code, open the command palette and run `Preferences: Open User Settings (JSON)`.

If debugging fails with `0xc0000135`, required runtime DLLs were not found at launch time. The provided `.vscode/launch.json` prepends `${config:cppdbg.msys2BinPath}` to `PATH` for debug sessions so GDB can run the executable with MSYS2/SDL dependencies available.

### Clangd Linting Setup in VSCode

Use https://github.com/nickdiego/compiledb to generate the json file clangd needs to debug.

Install with

```
pip install compiledb
# OR
python3 -m pip install compiledb
```

Then run

```
cd scripts
./compile-commands.sh
```

This should allow vscode /w clangd or other programs to ingest the file for usage in development.

### Emscripten

Install Emscripten using git.
```
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

### Levels

Levels used the Tiled map editor.  Simply edit the tmx files in the scratch dir, then run the tmx-to-levels.js file with node

```
node tmx-to-levels.js
```