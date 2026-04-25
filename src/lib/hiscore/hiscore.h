#pragma once

#include <string>
#include <vector>

namespace hiscore {

struct HiscoreRow {
  std::string name;
  int score;
  int maxCombo;
  int time;
};

std::vector<HiscoreRow> getHighScores();
std::vector<HiscoreRow> parseHiscoreText(const std::string& hiscoreText);

void saveHighScores(const std::vector<HiscoreRow>& hiscores);

}; // namespace hiscore