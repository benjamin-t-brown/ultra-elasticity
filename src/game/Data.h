#pragma once

#include "game/State.h"
#include <vector>

namespace program {

void loadLevelDataFromFile(
    std::unordered_map<std::string, std::unique_ptr<LevelData>>& levels);

bool isSmallBrick(BrickType brickType);

bool isSmallCircleBrick(BrickType brickType);

inline bool isMetalBrick(BrickType brickType) {
  return brickType == BRICK_TYPE_METAL || brickType == BRICK_TYPE_METAL_BOMB ||
         brickType == BRICK_TYPE_SMALL_SQUARE_METAL ||
         brickType == BRICK_TYPE_SMALL_CIRCLE_METAL;
}
inline bool isMetalBrickNoBomb(BrickType brickType) {
  return brickType == BRICK_TYPE_METAL ||
         brickType == BRICK_TYPE_SMALL_SQUARE_METAL ||
         brickType == BRICK_TYPE_SMALL_CIRCLE_METAL;
}
inline bool isVentBrick(BrickType brickType) {
  return brickType == BRICK_TYPE_VENT || brickType == BRICK_TYPE_SMALL_VENT;
}
inline bool isIndestructibleBrick(BrickType brickType) {
  return brickType == BRICK_TYPE_SMALL_INDESTRUCTIBLE;
}

BrickType levelBrickIdToBrickType(int brickId);

void generateBackgroundRow(std::vector<int>& row, int bgWidth);

void loadBackground(State& state, int terrainIndex);

void loadBricks(State& state, const std::vector<std::vector<int>>& brickIds);

void loadLevel(State& state, int levelIndex);

bool hasLevel(State& state, int levelIndex);

/// World-space vent air AABB (same geometry as ball vent force). `outHeadingDeg`
/// matches `physics::applyForce` for that zone.
void getVentAirZoneForFan(const State& state, const VentFan& fan,
                          physics::Rect& outR, double& outHeadingDeg);

} // namespace program
