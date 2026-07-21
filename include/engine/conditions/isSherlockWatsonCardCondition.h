#pragma once
#include "condition.h"
#include "model/card.h"
#include <algorithm>
#include <cctype>
#include <string>


class IsSherlockWatsonCardCondition : public Condition {
public:
  IsSherlockWatsonCardCondition() = default;
  ~IsSherlockWatsonCardCondition() override = default;

  bool check(gameData &gameData, Fighter *fighter = nullptr) override {
    (void)fighter;
    if (!gameData.cardPlayed) {
      return false;
    }
    std::string name = gameData.cardPlayed->getPerformerName();
    std::transform(name.begin(), name.end(), name.begin(),
                    [](unsigned char c) { return (char)std::tolower(c); });
    return name == "sherlock" || name == "watson";
  }
};
