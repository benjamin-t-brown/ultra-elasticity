#include "Game.h"
#include "Actor.h"
#include "Ball.h"
#include "Brick.h"
#include "GameOptions.h"
#include "LibHTML.h"
#include "Particle.h"
#include "Player.h"
#include "Powerup.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

const int GameOptions::width = 512;
const int GameOptions::height = 512;
const int GameOptions::spriteSize = 32;
const int GameOptions::playerSpeed = 3;

GameOptions::GameOptions() {}

float distance(const int x1, const int y1, const int x2, const int y2) {
  return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

// Helper: build terrain sprite name from the old SpriteList naming.
// terrain_0_0..3, terrain_1_0..3, terrain_2_0..3
static std::string terrainSpriteName(int terrainIndex, int bgValue) {
  return "terrain_" + std::to_string(terrainIndex) + "_" +
         std::to_string(bgValue);
}

Game::Game(sdl2w::Window& windowA)
    : shouldDrawMenu(true),
      shouldDrawRetry(false),
      shouldDrawRetryBlackOnly(false),
      shouldExit(false),
      shouldPlayHiscoreSound(false),
      isGameOver(false),
      levelIndex(0),
      score(0),
      scoreLevelStart(0),
      lastScore(0),
      combo(0),
      bricksXOffset(24 + 58 / 2 + 1),
      bricksYOffset(64 - 1),
      brickWidth(58),
      brickHeight(28),
      bgWidth((512 / 24 + 1)),
      bgHeight((512 / 24 + 2)),
      bgSpriteSize(24),
      bgOffset(0.0),
      terrainIndex(0),
      brickColor(""),
      isVictory(false),
      window(windowA),
      retryIndex(1),
      numRetries(0),
      startingGame(false) {

  player = std::make_unique<Player>(*this);

  sdl2w::Events& events = window.getEvents();
  events.setKeyboardEvent(sdl2w::ON_KEY_DOWN,
                          [this](const std::string& key, int) {
                            handleKeyDown(key);
                          });
  background.erase(background.begin(), background.end());
  for (unsigned int i = 0; i < bgHeight; i++) {
    std::vector<int> row = generateBackgroundRow();
    background.push_back(row);
  }
}

Game::~Game() {}

void Game::initPlayer() { player->set(256, 512 - 64); }

void Game::initWorld() {
  retryIndex = 1;
  isVictory = false;
  isGameOver = false;
  player->setPaddleState("normal");

  balls.erase(balls.begin(), balls.end());
  powerups.erase(powerups.begin(), powerups.end());
  bricks.erase(bricks.begin(), bricks.end());

  combo = 0;
  player->set(256, 512 - 64);
  player->setV(0, 0);
  addBall(256, 512 - 64 - 16, 0, 1);
  balls.back()->setSpeed(3.5);
  balls.back()->setSticky(true);

  if (levelIndex == 0) {
    brickColor = "";
    terrainIndex = 0;
    loadBricks({{0, 1, 1, 0, 0, 1, 1, 0},
                {1, 0, 1, 0, 0, 1, 0, 1},
                {1, 3, 0, 0, 0, 0, 3, 1},
                {1, 1, 1, 0, 0, 1, 1, 1},
                {0, 1, 1, 0, 0, 1, 1, 0}});
  } else if (levelIndex == 1) {
    brickColor = "blue";
    terrainIndex = 0;
    loadBricks({{2, 0, 1, 1, 1, 1, 0, 2},
                {2, 0, 1, 1, 4, 1, 0, 2},
                {2, 0, 1, 3, 1, 1, 0, 2},
                {2, 0, 0, 1, 1, 0, 0, 2},
                {2, 2, 5, 0, 0, 5, 2, 2}});
  } else if (levelIndex == 2) {
    brickColor = "green";
    terrainIndex = 0;
    loadBricks({{0, 1, 1, 1, 1, 1, 1, 0},
                {1, 4, 0, 1, 1, 0, 4, 1},
                {0, 1, 0, 2, 2, 0, 1, 0},
                {0, 1, 0, 2, 2, 0, 1, 0},
                {6, 2, 1, 4, 4, 1, 2, 6},
                {3, 2, 1, 0, 0, 1, 2, 3},
                {0, 2, 2, 1, 1, 2, 2, 0},
                {0, 0, 0, 1, 1, 0, 0, 0},
                {4, 0, 0, 2, 2, 0, 0, 4}});
  } else if (levelIndex == 3) {
    brickColor = "pink";
    terrainIndex = 2;
    loadBricks({{1, 0, 1, 2, 2, 1, 0, 1},
                {0, 1, 0, 1, 1, 0, 1, 0},
                {1, 2, 1, 0, 0, 1, 2, 1},
                {0, 1, 0, 1, 1, 0, 1, 0},
                {1, 2, 1, 0, 0, 1, 2, 1},
                {0, 1, 0, 1, 1, 0, 1, 0},
                {1, 0, 1, 3, 3, 1, 0, 1},
                {0, 1, 0, 1, 1, 0, 1, 0},
                {1, 0, 1, 1, 1, 1, 0, 1},
                {0, 1, 3, 0, 0, 3, 1, 0},
                {1, 0, 1, 1, 1, 1, 0, 1},
                {0, 1, 0, 1, 1, 0, 1, 0},
                {6, 1, 1, 2, 2, 1, 1, 6}});
  } else if (levelIndex == 4) {
    brickColor = "";
    terrainIndex = 2;
    loadBricks({{1, 0, 1, 0, 1, 0, 1, 0},
                {0, 1, 0, 1, 0, 1, 0, 1},
                {0, 1, 5, 4, 4, 5, 1, 0},
                {2, 3, 2, 0, 1, 2, 3, 2},
                {2, 2, 2, 1, 1, 2, 2, 2},
                {0, 1, 0, 1, 1, 0, 1, 0},
                {1, 0, 0, 4, 4, 0, 0, 1},
                {0, 1, 0, 1, 1, 0, 1, 0}});
  } else if (levelIndex == 5) {
    brickColor = "blue";
    terrainIndex = 2;
    loadBricks({
        {2, 5, 2, 1, 1, 2, 5, 2},
        {2, 0, 2, 4, 4, 2, 0, 2},
        {2, 1, 2, 2, 2, 2, 1, 2},
        {2, 0, 2, 4, 4, 2, 0, 2},
        {2, 1, 5, 2, 2, 5, 1, 2},
        {2, 0, 3, 4, 4, 3, 0, 2},
        {2, 1, 6, 1, 1, 6, 1, 2},
        {2, 2, 2, 4, 4, 2, 2, 2},
        {2, 2, 1, 1, 1, 1, 2, 2},
        {1, 1, 0, 0, 0, 0, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
    });
  } else if (levelIndex == 6) {
    brickColor = "green";
    terrainIndex = 1;
    loadBricks({
        {0, 4, 0, 4, 0, 4, 0, 4},
        {3, 0, 4, 0, 4, 0, 4, 0},
        {1, 1, 1, 1, 1, 1, 1, 6},
        {0, 4, 0, 2, 2, 4, 0, 4},
        {3, 0, 4, 2, 2, 0, 4, 0},
        {1, 1, 1, 1, 1, 1, 1, 6},
        {0, 4, 0, 4, 0, 4, 0, 4},
        {3, 0, 4, 0, 4, 0, 4, 0},
        {1, 1, 1, 1, 1, 1, 1, 6},
        {0, 4, 0, 4, 0, 4, 0, 4},
        {3, 0, 4, 0, 4, 0, 4, 0},
        {1, 1, 1, 1, 1, 1, 1, 6},
    });
  } else if (levelIndex == 7) {
    brickColor = "pink";
    terrainIndex = 1;
    loadBricks({
        {3, 4, 5, 0, 5, 4, 0, 3},
        {1, 0, 2, 0, 2, 0, 4, 5},
        {1, 1, 2, 0, 2, 1, 1, 1},
        {5, 2, 2, 0, 2, 4, 2, 5},
        {1, 0, 2, 0, 2, 0, 4, 0},
        {1, 1, 2, 0, 2, 1, 1, 5},
        {1, 4, 5, 0, 5, 4, 0, 4},
        {5, 0, 0, 0, 0, 0, 4, 5},
        {1, 1, 5, 0, 5, 1, 1, 6},
        {5, 1, 2, 0, 2, 4, 2, 5},
        {4, 1, 2, 0, 2, 0, 2, 4},
        {2, 2, 2, 0, 2, 1, 2, 2},
        {1, 0, 0, 0, 0, 0, 0, 1},
    });
  } else if (levelIndex == 8) {
    brickColor = "blue";
    terrainIndex = 1;
    loadBricks({{1, 1, 3, 0, 1, 3, 1, 0},
                {1, 3, 2, 1, 1, 2, 3, 1},
                {1, 2, 2, 1, 1, 2, 2, 1},
                {1, 2, 2, 1, 1, 2, 2, 1},
                {1, 1, 1, 0, 1, 1, 1, 0},
                {1, 2, 2, 1, 1, 2, 2, 1},
                {1, 2, 2, 1, 1, 2, 2, 1},
                {1, 2, 2, 1, 1, 2, 2, 1},
                {1, 1, 1, 0, 1, 1, 1, 0}});
  } else if (levelIndex == 9) {
    brickColor = "";
    terrainIndex = 1;
    loadBricks({
        {2, 2, 2, 5, 0, 2, 2, 2},
        {2, 3, 2, 0, 2, 5, 3, 2},
        {2, 1, 2, 0, 2, 3, 2, 2},
        {2, 2, 2, 3, 2, 4, 1, 2},
        {2, 0, 2, 0, 2, 2, 2, 2},
        {2, 1, 2, 0, 2, 3, 1, 2},
        {2, 3, 2, 0, 5, 4, 2, 2},
        {2, 0, 2, 0, 0, 0, 1, 2},
        {2, 1, 2, 0, 5, 1, 2, 2},
        {2, 1, 2, 0, 2, 4, 1, 2},
        {2, 1, 2, 0, 2, 0, 2, 2},
        {2, 2, 2, 2, 2, 1, 1, 2},
        {2, 6, 0, 0, 0, 0, 6, 2},
    });
  } else if (levelIndex == 10) {
    brickColor = "";
    terrainIndex = 1;
    loadBricks({
        {5, 2, 2, 2, 2, 2, 2, 2},
        {1, 0, 0, 0, 0, 0, 0, 2},
        {1, 2, 2, 2, 2, 2, 2, 2},
        {1, 2, 0, 0, 0, 0, 0, 0},
        {1, 2, 2, 2, 2, 2, 2, 2},
        {1, 0, 0, 0, 0, 0, 0, 2},
        {1, 2, 2, 2, 2, 2, 2, 2},
        {1, 2, 0, 0, 0, 0, 0, 0},
        {1, 2, 2, 2, 2, 2, 2, 2},
        {1, 0, 0, 0, 0, 0, 0, 2},
        {6, 2, 1, 2, 2, 2, 1, 2},
        {0, 2, 2, 2, 1, 2, 2, 2},
    });
  }

  auto [w, h] = window.getRenderDims();
  if (levelIndex == 10) {
    addTextParticle(w / 2, h / 2, "Final Level", 3000);
  } else {
    addTextParticle(w / 2, h / 2, "Level " + std::to_string(levelIndex + 1), 3000);
  }
  particles.back()->vy = 0;

  background.erase(background.begin(), background.end());
  for (unsigned int i = 0; i < bgHeight; i++) {
    std::vector<int> row = generateBackgroundRow();
    background.push_back(row);
  }

  player->startIntro();
}

void Game::loadBricks(const std::vector<std::vector<int>>& newBricks) {
  unsigned int height = newBricks.size();
  unsigned int width = newBricks[0].size();

  bricks.erase(bricks.begin(), bricks.end());
  for (unsigned int i = 0; i < height; i++) {
    for (unsigned int j = 0; j < width; j++) {
      int brickId = newBricks[i][j];
      if (brickId == 1) {
        bricks.push_back(std::make_unique<Brick>(*this, "normal"));
      } else if (brickId == 2) {
        bricks.push_back(std::make_unique<Brick>(*this, "metal"));
      } else if (brickId == 3) {
        bricks.push_back(std::make_unique<Brick>(*this, "powerup_good"));
      } else if (brickId == 4) {
        bricks.push_back(std::make_unique<Brick>(*this, "powerup_bad"));
      } else if (brickId == 5) {
        bricks.push_back(std::make_unique<Brick>(*this, "metalBomb"));
      } else if (brickId == 6) {
        bricks.push_back(std::make_unique<Brick>(*this, "powerup_metal"));
      }

      if (brickId > 0) {
        Brick& brick = *bricks[bricks.size() - 1];
        brick.set(bricksXOffset + j * brickWidth,
                  bricksYOffset + i * brickHeight);
      }
    }
  }
}

std::vector<int> Game::generateBackgroundRow() {
  std::vector<int> row;
  for (unsigned int i = 0; i < bgWidth; i++) {
    unsigned int x = i % bgWidth;
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
  return row;
}

void Game::enableMenu() {
  shouldDrawMenu = true;
  sdl2w::Events& events = window.getEvents();
  events.pushRoute();
  events.setKeyboardEvent(sdl2w::ON_KEY_DOWN,
                          [this](const std::string& key, int) {
                            handleKeyMenu(key);
                          });
  window.playMusic("menu");
}

void Game::disableMenu() {
  shouldDrawMenu = false;
  sdl2w::Events& events = window.getEvents();
  events.popRoute();
  window.stopMusic();
}

void Game::modifyScore(const int value) {
  if (shouldDrawMenu) {
    return;
  }
  score += value;
  addTextParticle(GameOptions::width / 2, 32, std::to_string(value), 500);
  particles.back()->vy = 0.5;
  if (score < 0) {
    score = 0;
  }
}

std::pair<double, double> Game::getNormalizedVec(const double x,
                                                 const double y) {
  double d = sqrt(x * x + y * y);
  return std::make_pair(x / d, y / d);
};

void Game::handleKeyDown(const std::string& /*key*/) {}

void Game::handleKeyUp(const std::string& /*key*/) {}

void Game::handleKeyUpdate() {
  const sdl2w::Events& events = window.getEvents();

  if (player->isPlayingIntro) {
    return;
  }

  if (events.isKeyPressed("Left")) {
    player->setDirection("left");
    player->setVx(-4);
    for (unsigned int i = 0; i < balls.size(); i++) {
      Ball& ball = *balls[i];
      if (ball.isSticky) {
        ball.setVx(-4);
      }
    }
  } else if (events.isKeyPressed("Right")) {
    player->setDirection("right");
    player->setVx(4);
    for (unsigned int i = 0; i < balls.size(); i++) {
      Ball& ball = *balls[i];
      if (ball.isSticky) {
        ball.setVx(4);
      }
    }
  } else {
    player->setDirection("up");
    player->setVx(0);
    for (unsigned int i = 0; i < balls.size(); i++) {
      Ball& ball = *balls[i];
      if (ball.isSticky) {
        ball.setVx(0);
      }
    }
  }

  if (events.isKeyPressed("Space") || events.isKeyPressed("Return")) {
    if (balls.size() > 0) {
      Ball& ball = *balls[0];
      ball.setSticky(false);
    }
  }
}

void Game::handleKeyMenu(const std::string& key) {
  if (key == "Escape") {
    shouldExit = true;
  } else {
    levelIndex = 0;
    score = 0;
    combo = 0;
    player->mana = 3;
    isGameOver = true;
    numRetries = 0;
    particles.push_back(std::make_unique<Particle>(*this, "fade_out", 1000));
    notifyGameStarted();
    addFuncTimer(1000, [&]() {
      disableMenu();
      terrainIndex = 0;
      window.playSound("game_start");
      particles.push_back(std::make_unique<Particle>(*this, "black", 2000));
      addFuncTimer(2000, [&]() {
        initPlayer();
        initWorld();
        particles.push_back(std::make_unique<Particle>(*this, "fade_in", 1000));
      });
    });
  }
}

void Game::handleKeyRetry(const std::string& key) {
  if (key == "Left") {
    if (retryIndex == 1) {
      retryIndex = 0;
    } else {
      retryIndex = 1;
    }
  } else if (key == "Right") {
    if (retryIndex == 1) {
      retryIndex = 0;
    } else {
      retryIndex = 1;
    }
  }

  if (key == "Return" || key == "Space" || key == "X" || key == "x") {
    sdl2w::Events& events = window.getEvents();
    events.popRoute();
    if (retryIndex == 0) {
      window.playSound("brick_hit1");
      shouldDrawRetryBlackOnly = true;
      addFuncTimer(500, [&]() {
        notifyGameCompleted(false, score, numRetries);
        shouldDrawRetry = false;
        shouldDrawRetryBlackOnly = false;
        enableMenu();
        particles.insert(particles.begin(),
                         std::make_unique<Particle>(*this, "fade_in", 500));
      });
    } else {
      score = scoreLevelStart;
      window.playSound("level_complete");
      startingGame = true;
      addFuncTimer(1500, [&]() {
        numRetries++;
        startingGame = false;
        shouldDrawRetry = false;
        initWorld();
        particles.push_back(std::make_unique<Particle>(*this, "fade_in", 1000));
      });
    }
  }
}

std::string Game::collidesCircleRect(const Circle& c, const Rect& r2) {
  Rect r1(c.x - c.r, c.y - c.r, c.r * 2, c.r * 2);
  double dx = (r1.x + r1.w / 2) - (r2.x + r2.w / 2);
  double dy = (r1.y + r1.h / 2) - (r2.y + r2.h / 2);
  double width = (r1.w + r2.w) / 2;
  double height = (r1.h + r2.h) / 2;
  double crossWidth = width * dy;
  double crossHeight = height * dx;
  std::string collision = "none";

  if (abs(static_cast<int>(dx)) <= width &&
      abs(static_cast<int>(dy)) <= height) {
    if (crossWidth > crossHeight) {
      collision = (crossWidth > (-crossHeight)) ? "bottom" : "left";
    } else {
      collision = (crossWidth > -(crossHeight)) ? "right" : "top";
    }

    if (c.x <= r2.x && c.y <= r2.y) {
      collision = "top-left";
    } else if (c.x >= r2.x + r2.w && c.y <= r2.y) {
      collision = "top-right";
    } else if (c.x <= r2.x && c.y >= r2.y + r2.h) {
      collision = "bottom-left";
    } else if (c.x >= r2.x + r2.w && c.y >= r2.y + r2.h) {
      collision = "bottom-right";
    }
  }

  return collision;
}

bool Game::collidesCircleCircle(const Circle& c1, const Circle& c2) {
  float d = distance(c1.x, c1.y, c2.x, c2.y);
  if (d < c1.r + c2.r) {
    return true;
  }
  return false;
}

void Game::checkPlayerBallCollision(Ball& ball, Player& player) {
  Circle ballCircle(ball.x, ball.y, ball.r);
  Circle playerCircle(player.x,
                      player.y + player.paddleCollisionRadius,
                      player.paddleCollisionRadius);

  if (ball.y > playerCircle.y - 8 || ball.vy < 0) {
    return;
  }

  if (player.isPaddleShort()) {
    if (ball.y > playerCircle.y - 32 ||
        ball.x < player.x - player.paddleShortCollisionRadius ||
        ball.x > player.x + player.paddleShortCollisionRadius) {
      return;
    }
    if (collidesCircleCircle(ballCircle, playerCircle)) {
      double normX = ball.x - playerCircle.x;
      double normY = ball.y - playerCircle.y;
      double nn = normX * normX + normY * normY;
      double vn = normX * ball.vx + normY * ball.vy;
      if (vn > 0) {
        return;
      }
      ball.set(ball.x - ball.vx, ball.y - ball.vy);
      ball.vx -= (2 * (vn / nn)) * normX;
      ball.vy -= (2 * (vn / nn)) * normY;
      player.handleCollision(ball);
    }
  } else {
    if (collidesCircleCircle(ballCircle, playerCircle)) {
      double normX = ball.x - playerCircle.x + player.vx;
      double normY = ball.y - playerCircle.y;
      double nn = normX * normX + normY * normY;
      double vn = normX * ball.vx + normY * ball.vy;
      if (vn > 0) {
        return;
      }
      ball.set(ball.x - ball.vx, ball.y - ball.vy);
      ball.vx -= (2 * (vn / nn)) * normX;
      ball.vy -= (2 * (vn / nn)) * normY;
      player.handleCollision(ball);
    }
  }
}

void Game::checkBallBrickCollision(Ball& ball, Brick& brick) {
  Circle ballCircle(ball.x, ball.y, ball.r);
  Rect brickRect(brick.x - brickWidth / 2,
                 brick.y - brickHeight / 2,
                 brickWidth,
                 brickHeight);

  std::string side = collidesCircleRect(ballCircle, brickRect);

  if (side != "none") {
    int indTop = brick.indexOfBrickOnSide("top");
    int indBot = brick.indexOfBrickOnSide("bottom");
    int indLeft = brick.indexOfBrickOnSide("left");
    int indRight = brick.indexOfBrickOnSide("right");
    if (side == "bottom-left") {
      if (indLeft > -1) {
        side = "bottom";
      } else if (indBot > -1) {
        side = "left";
      }
    } else if (side == "top-left") {
      if (indLeft > -1) {
        side = "top";
      } else if (indTop > -1) {
        side = "left";
      }
    } else if (side == "top-right") {
      if (indRight > -1) {
        side = "top";
      } else if (indTop > -1) {
        side = "right";
      }
    } else if (side == "bottom-right") {
      if (indRight > -1) {
        side = "bottom";
      } else if (indBot > -1) {
        side = "right";
      }
    }

    brick.handleCollision(ball);
    ball.handleCollision(brick, side);
  }
}

void Game::checkPlayerPowerupCollision(Powerup& powerup, Player& player) {
  Circle powerupCircle(powerup.x, powerup.y, powerup.r);
  Circle playerCircle(player.x,
                      player.y + player.paddleCollisionRadius,
                      player.paddleCollisionRadius);

  if (powerup.y > playerCircle.y - 32 +
                      static_cast<double>(player.paddleCollisionRadius) / 25) {
    return;
  }

  if (collidesCircleCircle(powerupCircle, playerCircle)) {
    player.handleCollision(powerup);
    powerup.remove();
  }
}

void Game::checkCollisions() {
  for (unsigned int i = 0; i < balls.size(); i++) {
    Ball& ball = *balls[i];
    checkPlayerBallCollision(ball, *player);

    for (int j = bricks.size() - 1; j >= 0; j--) {
      Brick& brick = *bricks[j];
      if (!brick.isExploding) {
        checkBallBrickCollision(ball, brick);
      }
    }
  }

  for (auto it = powerups.begin(); it != powerups.end(); ++it) {
    checkPlayerPowerupCollision(**it, *player);
  }
}

void Game::checkGameOver() {
  if (!isGameOver && balls.size() == 0) {
    isGameOver = true;
    window.playSound("game_over");
    addFuncTimer(2000, [&]() {
      sdl2w::Events& events = window.getEvents();
      events.pushRoute();
      events.setKeyboardEvent(sdl2w::ON_KEY_DOWN,
                              [this](const std::string& key, int) {
                                handleKeyRetry(key);
                              });
      shouldDrawRetry = true;
    });
  }

  bool hasMetalBall = false;
  for (unsigned int i = 0; i < balls.size(); i++) {
    Ball& ball = *balls[i];
    if (ball.ballType == "armored") {
      hasMetalBall = true;
    }
  }

  int numActiveBricks = 0;
  for (unsigned int i = 0; i < bricks.size(); i++) {
    Brick& brick = *bricks[i];
    if (hasMetalBall) {
      numActiveBricks++;
    } else if (brick.brickType != "metal") {
      numActiveBricks++;
    }
  }

  if (!isGameOver && numActiveBricks <= 0) {
    isGameOver = true;
    if (combo > 2) {
      modifyScore(combo * 75);
    }
    addFuncTimer(250, [&]() {
      window.playSound("level_complete");
      particles.insert(particles.begin(),
                       std::make_unique<Particle>(*this, "fade_out", 1000));
      addFuncTimer(1000, [&]() {
        levelIndex++;
        int ms = 2000;
        if (levelIndex == 11) {
          ms = 15000;
        }
        addTextParticle(
            GameOptions::width / 2,
            GameOptions::height / 2,
            "Remaining ball bonus: " + std::to_string(balls.size()) + "x 1000");
        modifyScore(balls.size() * 1000);
        scoreLevelStart = score;
        particles.insert(particles.begin(),
                         std::make_unique<Particle>(*this, "black", ms));
        addFuncTimer(2000, [&]() {
          if (levelIndex == 11) {
            window.playSound("victory");
            isVictory = true;
            addFuncTimer(15000, [&]() {
              isVictory = false;
              notifyGameCompleted(true, score, numRetries);
              enableMenu();
              particles.push_back(
                  std::make_unique<Particle>(*this, "fade_in", 1000));
            });
          } else {
            initWorld();
            particles.push_back(
                std::make_unique<Particle>(*this, "fade_in", 1000));
          }
        });
      });
    });
  }
}

void Game::addBoolTimer(const int ms, bool& ref) {
  timers.push_back(std::make_unique<BoolTimer>(ms, ref));
}

void Game::addFuncTimer(const int ms, std::function<void()> cb) {
  timers.push_back(std::make_unique<FuncTimer>(ms, cb));
}

void Game::addPowerup(int x, int y, const std::string& type) {
  powerups.push_back(std::make_unique<Powerup>(*this, type));
  powerups.back()->set(x, y);
}

void Game::addTextParticle(int x, int y, const std::string& text, int ms) {
  particles.push_back(std::make_unique<Particle>(*this, "text", ms));
  particles.back()->text = text;
  particles.back()->set(x, y);
}

void Game::addBall(int x, int y, double vx, double vy) {
  balls.push_back(std::make_unique<Ball>(*this));
  Ball& ball = *balls.back();
  ball.set(x, y);
  ball.setV(vx, vy);
}

void Game::drawTerrain() {
  sdl2w::Draw& d = window.getDraw();
  for (unsigned int i = 0; i < background.size(); i++) {
    for (unsigned int j = 0; j < background[i].size(); j++) {
      int x = j * bgSpriteSize;
      int y = i * bgSpriteSize - bgSpriteSize * 0 + bgOffset - 24;
      std::string spriteName = terrainSpriteName(terrainIndex, background[i][j]);
      d.drawSprite(window.getStore().getSprite(spriteName),
                   sdl2w::RenderableParams{.x = x, .y = y, .centered = false});
    }
  }

  double dBgSpriteSize = static_cast<double>(bgSpriteSize);
  auto [w, h] = window.getRenderDims();
  (void)w;
  bgOffset += 0.5 * (window.getDeltaTime() / (1000.0 / 60.0));
  if (bgOffset > dBgSpriteSize) {
    bgOffset = bgOffset - dBgSpriteSize;
    background.pop_back();
    std::vector<int> row = generateBackgroundRow();
    background.insert(background.begin(), 1, row);
  }
}

void Game::drawMenu() {
  sdl2w::Draw& d = window.getDraw();
  auto [w, h] = window.getRenderDims();
  int titleX = w / 2;
  int titleY = h / 2 - 64;
  d.drawText(GameOptions::programName,
             sdl2w::RenderTextParams{.fontName = "default",
                                     .fontSize = sdl2w::TextSize::TEXT_SIZE_72,
                                     .x = titleX,
                                     .y = titleY,
                                     .color = {255, 255, 255, 255},
                                     .centered = true});

  int startTextX = w / 2;
  int startTextY = h - h / 4;
  d.drawText("Press button to start.",
             sdl2w::RenderTextParams{.fontName = "default",
                                     .fontSize = sdl2w::TextSize::TEXT_SIZE_36,
                                     .x = startTextX,
                                     .y = startTextY,
                                     .color = {255, 255, 255, 255},
                                     .centered = true});

  if (score) {
    int scoreTextX = w / 2;
    int scoreTextY = h - h / 3;
    d.drawText("Last Score: " + std::to_string(score),
               sdl2w::RenderTextParams{.fontName = "default",
                                       .fontSize = sdl2w::TextSize::TEXT_SIZE_18,
                                       .x = scoreTextX,
                                       .y = scoreTextY,
                                       .color = {255, 255, 255, 255},
                                       .centered = true});
  }
}

void Game::drawRetry() {
  sdl2w::Draw& d = window.getDraw();
  auto [w, h] = window.getRenderDims();
  d.drawSprite(window.getStore().getSprite("cpp_splash_black"),
               sdl2w::RenderableParams{.x = 0, .y = 0, .centered = false});
  if (shouldDrawRetryBlackOnly) {
    return;
  }

  int titleX = w / 2;
  int titleY = h / 2;
  d.drawText("Retry?",
             sdl2w::RenderTextParams{.fontName = "default",
                                     .fontSize = sdl2w::TextSize::TEXT_SIZE_36,
                                     .x = titleX,
                                     .y = titleY,
                                     .color = {255, 255, 255, 255},
                                     .centered = true});

  d.drawText("No.",
             sdl2w::RenderTextParams{.fontName = "default",
                                     .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                                     .x = w / 3,
                                     .y = h - h / 3,
                                     .color = {255, 255, 255, 255},
                                     .centered = true});
  SDL_Color yesColor = startingGame ? SDL_Color{255, 0, 0, 255}
                                    : SDL_Color{255, 255, 255, 255};
  d.drawText("Yes.",
             sdl2w::RenderTextParams{.fontName = "default",
                                     .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                                     .x = w - w / 3,
                                     .y = h - h / 3,
                                     .color = yesColor,
                                     .centered = true});

  if (retryIndex == 0) {
    d.drawSprite(window.getStore().getSprite("mana_0"),
                 sdl2w::RenderableParams{.x = w / 3,
                                         .y = h - h / 3 + 32});
  } else {
    d.drawSprite(window.getStore().getSprite("mana_0"),
                 sdl2w::RenderableParams{.x = w - w / 3,
                                         .y = h - h / 3 + 32});
  }
}

void Game::drawUI() {
  sdl2w::Draw& d = window.getDraw();
  auto [w, h] = window.getRenderDims();
  d.drawText("Combo: " + std::to_string(combo),
             sdl2w::RenderTextParams{.fontName = "default",
                                     .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                                     .x = 16,
                                     .y = 4,
                                     .color = {255, 255, 128, 255},
                                     .centered = false});

  if (player->isPlayingIntro) {
    d.drawSprite(window.getStore().getSprite("cpp_splash_black"),
                 sdl2w::RenderableParams{.x = 0, .y = h - 28, .centered = false});
    d.drawSprite(window.getStore().getSprite("cpp_splash_black"),
                 sdl2w::RenderableParams{.x = 0, .y = -h + 32, .centered = false});
    player->draw();
  }
}

bool Game::menuLoop() {
  drawTerrain();
  drawMenu();

  for (unsigned int i = 0; i < particles.size(); i++) {
    Particle& particle = *particles[i];
    particle.update();
    particle.draw();
    if (particle.shouldRemove()) {
      particles.erase(particles.begin() + i);
      i--;
    }
  }

  return !shouldExit;
}

bool Game::gameLoop() {
  handleKeyUpdate();

  drawTerrain();

  player->update();
  player->draw();

  for (unsigned int i = 0; i < bricks.size(); i++) {
    Brick& brick = *bricks[i];
    if (!isGameOver)
      brick.update();
    brick.draw();
    if (brick.shouldRemove()) {
      bricks.erase(bricks.begin() + i);
      i--;
    }
  }

  for (unsigned int i = 0; i < balls.size(); i++) {
    Ball& ball = *balls[i];
    if (!isGameOver)
      ball.update();
    ball.draw();
    if (ball.shouldRemove()) {
      balls.erase(balls.begin() + i);
      i--;
    }
  }

  for (unsigned int i = 0; i < powerups.size(); i++) {
    Powerup& powerup = *powerups[i];
    if (!isGameOver)
      powerup.update();
    powerup.draw();
    if (powerup.shouldRemove()) {
      powerups.erase(powerups.begin() + i);
      i--;
    }
  }

  drawUI();

  for (unsigned int i = 0; i < particles.size(); i++) {
    Particle& particle = *particles[i];
    particle.update();
    particle.draw();
    if (particle.shouldRemove()) {
      particles.erase(particles.begin() + i);
      i--;
    }
  }

  checkCollisions();
  checkGameOver();

  sdl2w::Draw& d = window.getDraw();
  auto [w, h] = window.getRenderDims();

  if (isVictory) {
    int titleX = w / 2;
    int titleY = h / 2;
    d.drawText("VICTORY!",
               sdl2w::RenderTextParams{.fontName = "default",
                                       .fontSize = sdl2w::TextSize::TEXT_SIZE_72,
                                       .x = titleX,
                                       .y = titleY,
                                       .color = {255, 255, 255, 255},
                                       .centered = true});
    d.drawText("Retries used: " + std::to_string(numRetries),
               sdl2w::RenderTextParams{.fontName = "default",
                                       .fontSize = sdl2w::TextSize::TEXT_SIZE_18,
                                       .x = w / 2,
                                       .y = h - 96,
                                       .color = {255, 0, 0, 255},
                                       .centered = true});
    d.drawText("Score: " + std::to_string(score),
               sdl2w::RenderTextParams{.fontName = "default",
                                       .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                                       .x = w / 2,
                                       .y = h - 64,
                                       .color = {255, 255, 255, 255},
                                       .centered = true});
  } else {
    d.drawText("Score: " + std::to_string(score),
               sdl2w::RenderTextParams{.fontName = "default",
                                       .fontSize = sdl2w::TextSize::TEXT_SIZE_20,
                                       .x = w / 2,
                                       .y = 16,
                                       .color = {255, 255, 255, 255},
                                       .centered = true});
  }

  return !shouldExit;
}

bool Game::loop() {
  int dt = window.getDeltaTime();
  for (unsigned int i = 0; i < timers.size(); i++) {
    Timer& timer = *timers[i];
    timer.update(dt);
    if (timer.shouldRemove()) {
      timers.erase(timers.begin() + i);
      i--;
    }
  }

  if (shouldPlayHiscoreSound) {
    shouldPlayHiscoreSound = false;
  }

  if (shouldDrawRetry) {
    drawRetry();
    return true;
  } else if (shouldDrawMenu) {
    return menuLoop();
  } else {
    return gameLoop();
  }
}

void Game::handleGameOver() {}
