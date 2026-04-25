EXE=ULTRAELASTICITY
EXE_SUFFIX=

MAIN_ALL=main.cpp

CODE=\
client/Render.cpp\
client/UiRenderer.cpp\
client/Components/EndGameScreen.cpp\
client/Components/InterLevelScreen.cpp\
game/Data.cpp\
game/State.cpp\
game/GameManager.cpp\
game/Physics.cpp\
game/updaters/CheckCollisions.cpp\
game/updaters/UpdateBackground.cpp\
game/updaters/UpdateEntities.cpp\
game/updaters/UpdatePlayer.cpp

LIB_CODE=\
lib/hiscore/hiscore.cpp\

TARGET ?= native
BASE_BUILD_DIR = build

SDL2W_REPO_URL = https://github.com/benjamin-t-brown/sdl2w.git
SDL2W_DIR_NAME = ../sdl2w
SDL2W_BUILD_OUTPUT_DIR = $(SDL2W_DIR_NAME)/sdl2w

MAIN_ALL_OBJECT=$(MAIN_ALL:.cpp=.o)

OBJECTS=$(CODE:.cpp=.o)
LIB_OBJECTS=$(LIB_CODE:.cpp=.o)

DEPENDS := $(patsubst %.cpp,%.d,$(CODE)) main.d
LIB_DEPENDS := $(patsubst %.cpp,%.d,$(LIB_CODE))

SDL2W_STAMP = $(SDL2W_DIR_NAME)/.source_ok
SDL2W_LIB = lib/sdl2w/libsdl2w.a

EMCC_LIBS=\
-s USE_SDL=2\
-s USE_SDL_IMAGE=2\
-s USE_SDL_MIXER=2\
-s SDL2_IMAGE_FORMATS='["png"]'\
-s USE_SDL_TTF=2\
-s USE_SDL_GFX=2\
-s ALLOW_MEMORY_GROWTH=1\
-s SAFE_HEAP=0\
-s ASSERTIONS=1\
-s INITIAL_MEMORY=326565888\
-s ENVIRONMENT=web\
-s DISABLE_EXCEPTION_CATCHING=0\
-Llib/sdl2w -lsdl2w -lidbfs.js

EMCC_EXPORTED=\
-s EXPORTED_FUNCTIONS='[\
 "_main",\
 "_enableSound",\
 "_disableSound",\
 "_setVolume",\
 "_setKeyDown",\
 "_setKeyUp",\
 "_setKeyStatus",\
 "_sendEvent"\
]'\
-s EXPORTED_RUNTIME_METHODS=['ccall']

DEBUG=true
LIBS=-Llib/sdl2w
INCLUDES= -I.
ifeq ($(TARGET),wasm)
    FLAGS = -Wall -std=c++17 -Oz
    LIBS = $(EMCC_LIBS) $(EMCC_EXPORTED) --preload-file assets
    EXE_SUFFIX=.js
    CXX = em++
else
    FLAGS = -O0 -Wall -std=c++17 -fno-omit-frame-pointer
    CXX = g++
    ifeq ($(OS),Windows_NT)
        LIBS += -lsdl2w -mconsole -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
    else
        UNAME_S := $(shell uname -s)
        ifeq ($(UNAME_S),Darwin)
            LIBS += -L/opt/homebrew/lib -L/usr/local/lib -lsdl2w -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
            INCLUDES += -I/opt/homebrew/include -I/usr/local/include
        else
            LIBS += -lsdl2w -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
        endif
    endif
    ifeq ($(DEBUG),true)
        FLAGS += -g
    endif
endif

.PHONY: all sdl2w_wasm sdl2w-rebuild _copy_sdl2w_artifacts

all: $(EXE)

$(EXE): $(OBJECTS) $(LIB_OBJECTS) $(MAIN_ALL_OBJECT) $(SDL2W_LIB)
	$(CXX) $(FLAGS) $(INCLUDES) $(MAIN_ALL_OBJECT) $(OBJECTS) $(LIB_OBJECTS) -o $(EXE)$(EXE_SUFFIX) $(LIBS)

# Convenience alias: builds $(SDL2W_LIB) only when needed (see rule below).
sdl2w: $(SDL2W_LIB)

# Force a full sdl2w rebuild + copy (use after upgrading sdl2w or changing its build).
sdl2w_rebuild:
	$(MAKE) -C "$(SDL2W_DIR_NAME)/src" -j8 all
	@$(MAKE) _copy_sdl2w_artifacts

$(SDL2W_STAMP):
	@if [ ! -d $(SDL2W_DIR_NAME) ]; then \
		echo "Directory $(SDL2W_DIR_NAME) not found. Cloning $(SDL2W_REPO_URL) into $(SDL2W_DIR_NAME)..."; \
		git clone $(SDL2W_REPO_URL) $(SDL2W_DIR_NAME); \
	else \
		echo "Directory $(SDL2W_DIR_NAME) found. Assuming it is the sdl2w repository."; \
	fi
	@touch $@

$(SDL2W_LIB): $(SDL2W_STAMP)
	@echo "Building standard sdl2w library (running 'make all' in $(SDL2W_DIR_NAME)/src)...";
	$(MAKE) -C "$(SDL2W_DIR_NAME)/src" -j8 all
	@$(MAKE) _copy_sdl2w_artifacts

sdl2w_wasm: $(SDL2W_STAMP)
	@echo "Building sdl2w library for WebAssembly (running 'make wasm' in $(SDL2W_DIR_NAME)/src)...";
	$(MAKE) -C "$(SDL2W_DIR_NAME)/src" -j8 wasm "FLAGS=-Wall -std=c++17 -Oz"
	@$(MAKE) _copy_sdl2w_artifacts

_copy_sdl2w_artifacts:
	@mkdir -p lib/sdl2w
	@echo "Copying sdl2w artifacts from $(SDL2W_BUILD_OUTPUT_DIR) to lib/sdl2w..."
	@cp -uv $(SDL2W_BUILD_OUTPUT_DIR)/lib/libsdl2w.a lib/sdl2w
	@cp -uv $(SDL2W_BUILD_OUTPUT_DIR)/include/*.h lib/sdl2w

-include $(DEPENDS)
-include $(LIB_DEPENDS)

%.o: %.cpp
	$(CXX) $(FLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

run:
	./$(EXE)

js: sdl2w_wasm
	$(MAKE) TARGET=wasm -j8 $(EXE)
	mv $(EXE).js ../web/
	mv $(EXE).data ../web/
	mv $(EXE).wasm ../web/

debug:
	gdb $(EXE)

clean:
	rm -f $(OBJECTS) $(LIB_OBJECTS) $(MAIN_ALL_OBJECT)
	rm -f $(DEPENDS) $(LIB_DEPENDS)
	rm -f $(EXE) $(EXE).exe
	rm -rf main.d
	rm -rf .build
	# rm -rf lib/sdl2w
	rm -rf $(SDL2W_BUILD_OUTPUT_DIR)
	rm -rf $(SDL2W_DIR_NAME)/src/build/
	rm -rf ../dist/$(EXE).*
