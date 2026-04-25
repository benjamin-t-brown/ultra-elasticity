#include "game/Data.h"
#include "lib/sdl2w/AssetLoader.h"
#include "lib/sdl2w/Logger.h"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

namespace program {

std::string loadTextFile(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    return "";
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

std::string loadLevelsText() {
  const std::vector<std::string> candidatePaths = {
      "assets/levels.txt",
  };
  for (const auto& path : candidatePaths) {
    std::string data = loadTextFile(path);
    if (!data.empty()) {
      return data;
    }
  }
  sdl2w::Logger::throwRuntimeError(
      "Unable to load levels data file from known paths.",
      __FILE__,
      __LINE__);
  return "";
}

struct ParsedLevelToken {
  int levelId = 0;
  int directionId = 30;
};

ParsedLevelToken parseLevelToken(const std::string& token) {
  ParsedLevelToken parsed;
  size_t delimPos = token.find(':');
  std::string levelIdToken = token;
  if (delimPos != std::string::npos) {
    levelIdToken = token.substr(0, delimPos);
  }
  parsed.levelId = std::stoi(levelIdToken);
  if (delimPos == std::string::npos) {
    return parsed;
  }
  std::string directionToken = token.substr(delimPos + 1);
  if (directionToken.empty()) {
    return parsed;
  }
  int directionId = std::stoi(directionToken);
  parsed.directionId =
      (directionId == 30 || directionId == 31 || directionId == 32 ||
       directionId == 33 || directionId == 34 || directionId == 35 ||
       directionId == 36)
          ? directionId
          : 30;
  return parsed;
}

BrickType levelBrickIdToBrickType(int brickId) {
  switch (brickId) {
  case BRICK_TYPE_NONE:
    return BRICK_TYPE_NONE;
  case BRICK_TYPE_NORMAL:
    return BRICK_TYPE_NORMAL;
  case BRICK_TYPE_METAL:
    return BRICK_TYPE_METAL;
  case BRICK_TYPE_METAL_BOMB:
    return BRICK_TYPE_METAL_BOMB;
  case BRICK_TYPE_POWERUP_EXTRA_BALLS:
    return BRICK_TYPE_POWERUP_EXTRA_BALLS;
  case BRICK_TYPE_POWERUP_SHORT_PADDLE:
    return BRICK_TYPE_POWERUP_SHORT_PADDLE;
  case BRICK_TYPE_POWERUP_ARMOR:
    return BRICK_TYPE_POWERUP_ARMOR;
  case BRICK_TYPE_POWERUP_TIMER:
    return BRICK_TYPE_POWERUP_TIMER;
  case BRICK_TYPE_POWERUP_METAL_BALLS:
    return BRICK_TYPE_POWERUP_METAL_BALLS;
  case BRICK_TYPE_BRITTLE:
    return BRICK_TYPE_BRITTLE;
  case BRICK_TYPE_SMALL_SQUARE_NORMAL:
    return BRICK_TYPE_SMALL_SQUARE_NORMAL;
  case BRICK_TYPE_SMALL_CIRCLE_NORMAL:
    return BRICK_TYPE_SMALL_CIRCLE_NORMAL;
  case BRICK_TYPE_SMALL_SQUARE_METAL:
    return BRICK_TYPE_SMALL_SQUARE_METAL;
  case BRICK_TYPE_SMALL_CIRCLE_METAL:
    return BRICK_TYPE_SMALL_CIRCLE_METAL;
  case BRICK_TYPE_VENT:
    return BRICK_TYPE_VENT;
  case BRICK_TYPE_SMALL_VENT:
    return BRICK_TYPE_SMALL_VENT;
  case BRICK_TYPE_SMALL_INDESTRUCTIBLE:
    return BRICK_TYPE_SMALL_INDESTRUCTIBLE;
  default:
    LOG(ERROR) << "Invalid brick id: " << brickId << LOG_ENDL;
    return BRICK_TYPE_NONE;
  }
}

void generateBackgroundRow(std::vector<int>& row, int bgWidth) {
  for (int i = 0; i < bgWidth; i++) {
    int x = i % bgWidth;
    if (x < 4 || x > bgWidth - 5) {
      row.push_back(2 + rand() % 2);
    } else if (x < 5 || x > bgWidth - 7) {
      row.push_back(1 + rand() % 3);
    } else if (x < 8 || x > bgWidth - 9) {
      row.push_back(rand() % 3);
    } else {
      row.push_back(rand() % 2);
    }
  }
}

void loadBackground(State& state, int terrainIndex) {
  state.background.terrainIndex = terrainIndex;
  state.background.rows.erase(state.background.rows.begin(),
                              state.background.rows.end());
  for (int i = 0; i < static_cast<int>(state.background.bgHeight); i++) {
    state.background.rows.push_back(std::vector<int>());
    generateBackgroundRow(state.background.rows.back(),
                          state.background.bgWidth);
  }
}

void loadBricks(State& state, const std::vector<std::vector<int>>& brickIds) {
  unsigned int height = brickIds.size();
  unsigned int width = brickIds[0].size();

  for (unsigned int i = 0; i < height; i++) {
    for (unsigned int j = 0; j < width; j++) {
      int brickId = brickIds[i][j];
      BrickType brickType = levelBrickIdToBrickType(brickId);
      if (brickType != BRICK_TYPE_NONE) {
        state.bricks.push_back(std::unique_ptr<Brick>(new Brick{}));
        Brick& brick = *state.bricks.back();
        brick.brickType = brickType;
        brick.coll.x =
            state.levelInfo.bricksXOffset + j * state.levelInfo.brickWidth;
        brick.coll.y =
            state.levelInfo.bricksYOffset + i * state.levelInfo.brickHeight;
        brick.coll.w = state.levelInfo.brickWidth;
        brick.coll.h = state.levelInfo.brickHeight;
        brick.physics.x = brick.coll.x;
        brick.physics.y = brick.coll.y;
        ;
      }
    }
  }
}

bool isSmallBrick(BrickType brickType) {
  return brickType == BRICK_TYPE_SMALL_SQUARE_NORMAL ||
         brickType == BRICK_TYPE_SMALL_CIRCLE_NORMAL ||
         brickType == BRICK_TYPE_SMALL_SQUARE_METAL ||
         brickType == BRICK_TYPE_SMALL_CIRCLE_METAL ||
         brickType == BRICK_TYPE_SMALL_VENT ||
         brickType == BRICK_TYPE_SMALL_INDESTRUCTIBLE ||
         brickType == BRICK_TYPE_NONE;
}
bool isSmallCircleBrick(BrickType brickType) {
  return brickType == BRICK_TYPE_SMALL_CIRCLE_NORMAL ||
         brickType == BRICK_TYPE_SMALL_CIRCLE_METAL;
}

void loadLevel(State& state, int levelIndex) {
  int terrainIndex = 0;
  std::string levelName = "level" + std::to_string(levelIndex);
  auto it = state.levels.find(levelName);
  if (it == state.levels.end()) {
    sdl2w::Logger::throwRuntimeError(
        "Level not found: " + levelName, __FILE__, __LINE__);
  }
  auto& levelData = it->second;
  BrickColor brickColor = levelData->backgroundInd == 0   ? BRICK_COLOR_DEFAULT
                          : levelData->backgroundInd == 1 ? BRICK_COLOR_BLUE
                                                          : BRICK_COLOR_GREEN;
  terrainIndex = levelData->backgroundInd;
  state.terrainIndex = terrainIndex;
  state.fans = levelData->fans;
  state.levelInfo.bricksXOffset =
      state.levelInfo.playAreaWidth / 2 -
      (levelData->width * (state.levelInfo.brickWidth / 2)) / 2;
  for (int i = 0; i < levelData->height; i++) {
    for (int j = 0; j < levelData->width; j++) {
      int brickId = levelData->bricks[i * levelData->width + j];
      BrickType brickType = levelBrickIdToBrickType(brickId);
      if (brickType != BRICK_TYPE_NONE) {
        double brickWidth =
            isSmallBrick(brickType)
                ? static_cast<double>(state.levelInfo.brickWidth) / 2.
                : state.levelInfo.brickWidth;
        int levelTileWidth = state.levelInfo.brickWidth / 2.;
        state.bricks.push_back(std::unique_ptr<Brick>(new Brick{}));
        Brick& brick = *state.bricks.back();
        brick.brickType = brickType;
        brick.coll.x = state.levelInfo.bricksXOffset + j * levelTileWidth;
        brick.coll.y =
            state.levelInfo.bricksYOffset + i * state.levelInfo.brickHeight;
        brick.coll.w = brickWidth;
        brick.coll.h = state.levelInfo.brickHeight;
        brick.circleColl.x = brick.coll.x + brickWidth / 2.;
        brick.circleColl.y = brick.coll.y + state.levelInfo.brickHeight / 2.;
        brick.circleColl.r = state.levelInfo.brickRadius;
        brick.physics.x = brick.coll.x;
        brick.physics.y = brick.coll.y;
      }
      if (!isSmallBrick(brickType)) {
        j++;
      }
    }
  }

  state.levelInfo.brickColor = brickColor;
  state.terrainIndex = terrainIndex;
}

void loadLevelDataFromFile(
    std::unordered_map<std::string, std::unique_ptr<LevelData>>& levels) {
  std::string levelData = loadLevelsText();
  std::vector<std::string> lines;
  sdl2w::split(levelData, "\n", lines);
  std::string currentLevelName = "";
  for (int i = 0; i < static_cast<int>(lines.size()); i++) {
    const std::string& line = lines[i];

    if (line.empty()) {
      continue;
    }
    if (line[0] == '#') {
      LevelData* newLevel = new LevelData();

      if (i + 2 >= static_cast<int>(lines.size())) {
        sdl2w::Logger::throwRuntimeError(
            "Invalid levels.txt data.", __FILE__, __LINE__);
      }

      const std::string& nextLine = lines[i + 1];
      const std::string& nextNextLine = lines[i + 2];
      newLevel->name = nextLine;
      std::vector<std::string> dimsArr;
      sdl2w::split(nextNextLine, ",", dimsArr);
      if (dimsArr.size() != 2) {
        sdl2w::Logger::throwRuntimeError(
            "Invalid levels.txt data: width and height must be specified.",
            __FILE__,
            __LINE__);
      }
      newLevel->width = std::stoi(dimsArr[0]);
      newLevel->height = std::stoi(dimsArr[1]);
      newLevel->backgroundInd = (static_cast<int>(levels.size()) / 3) % 3;
      i += 2;
      levels[newLevel->name] = std::unique_ptr<LevelData>(newLevel);
      currentLevelName = newLevel->name;
      LOG(INFO) << "Loaded level: " << newLevel->name << LOG_ENDL;
      continue;
    }
    std::vector<std::string> levelIds;
    sdl2w::split(line, ",", levelIds);
    auto it = levels.find(currentLevelName);
    if (it == levels.end()) {
      sdl2w::Logger::throwRuntimeError(
          "Current level not found: " + currentLevelName, __FILE__, __LINE__);
    }
    auto& level = it->second;
    for (int j = 0; j < static_cast<int>(levelIds.size()); j++) {
      int tileIndex = static_cast<int>(level->bricks.size());
      int x = tileIndex % level->width;
      int y = tileIndex / level->width;
      ParsedLevelToken parsedToken = parseLevelToken(levelIds[j]);
      BrickType brickType = levelBrickIdToBrickType(parsedToken.levelId);
      level->bricks.push_back(brickType);
      if (brickType == BRICK_TYPE_VENT || brickType == BRICK_TYPE_SMALL_VENT) {
        bool isRightHalfOfLargeVent = false;
        if (brickType == BRICK_TYPE_VENT && x > 0 && j > 0) {
          ParsedLevelToken parsedPrevToken = parseLevelToken(levelIds[j - 1]);
          BrickType prevBrickType =
              levelBrickIdToBrickType(parsedPrevToken.levelId);
          isRightHalfOfLargeVent =
              prevBrickType == BRICK_TYPE_VENT &&
              parsedPrevToken.directionId == parsedToken.directionId;
        }
        VentFan fan;
        fan.x = x;
        fan.y = y;
        fan.direction = parsedToken.directionId;
        if (brickType == BRICK_TYPE_SMALL_VENT) {
          fan.spanTiles = 1;
        } else {
          fan.spanTiles = isRightHalfOfLargeVent ? 1 : 2;
        }
        level->fans.push_back(fan);
      }
    }
  }
}

bool hasLevel(State& state, int levelIndex) {
  std::string levelName = "level" + std::to_string(levelIndex);
  auto it = state.levels.find(levelName);
  if (it == state.levels.end()) {
    return false;
  }
  return true;
}

void getVentAirZoneForFan(const State& state,
                          const VentFan& fan,
                          physics::Rect& outR,
                          double& outHeadingDeg) {
  constexpr double kVentAirExtendH = 2.0;
  constexpr double kVentAirExtendW = 2.0;
  const double tw = static_cast<double>(state.levelInfo.brickWidth) / 2.0;
  const double bh = static_cast<double>(state.levelInfo.brickHeight);
  const double offX = static_cast<double>(state.levelInfo.bricksXOffset);
  const double offY = static_cast<double>(state.levelInfo.bricksYOffset);
  const double ventLeft = offX + static_cast<double>(fan.x) * tw;
  const double ventTop = offY + static_cast<double>(fan.y) * bh;
  const int spanT = fan.spanTiles > 0 ? fan.spanTiles : 1;
  const double ventW = static_cast<double>(spanT) * tw;

  switch (fan.direction) {
  case 30: { // up
    outR = {
        ventLeft, ventTop - kVentAirExtendH * bh, ventW, kVentAirExtendH * bh};
    outHeadingDeg = 0.;
    break;
  }
  case 31: { // right
    outR = {ventLeft + ventW, ventTop, kVentAirExtendW * tw, bh};
    outHeadingDeg = 90.;
    break;
  }
  case 32: { // left
    outR = {ventLeft - kVentAirExtendW * tw, ventTop, kVentAirExtendW * tw, bh};
    outHeadingDeg = 270.;
    break;
  }
  case 33: { // up-right
    outR = {ventLeft + ventW - 2.0 * tw,
            ventTop - kVentAirExtendH * bh,
            2.0 * tw,
            kVentAirExtendH * bh};
    outHeadingDeg = 45.;
    break;
  }
  case 34: { // up-left
    outR = {ventLeft - 2.0 * tw,
            ventTop - kVentAirExtendH * bh,
            2.0 * tw,
            kVentAirExtendH * bh};
    outHeadingDeg = 315.;
    break;
  }
  case 35: { // down-right
    outR = {ventLeft + ventW - 2.0 * tw,
            ventTop + 0.5 * bh,
            2.0 * tw,
            kVentAirExtendH * bh};
    outHeadingDeg = 135.;
    break;
  }
  case 36: { // down-left
    outR = {ventLeft - 2.0 * tw,
            ventTop + 0.5 * bh,
            2.0 * tw,
            kVentAirExtendH * bh};
    outHeadingDeg = 225.;
    break;
  }
  default: {
    outR = {
        ventLeft, ventTop - kVentAirExtendH * bh, ventW, kVentAirExtendH * bh};
    outHeadingDeg = 0.;
    break;
  }
  }
}

} // namespace program
