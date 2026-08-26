#pragma once
#include "condition.h"
#include "model/fighter.h"
#include <algorithm>
#include <cctype>
#include <string>

class IsSherlockWatsonCardCondition : public Condition { // ability of sherlock
public:
  IsSherlockWatsonCardCondition() = default;
  ~IsSherlockWatsonCardCondition() override = default;

  bool check(gameData &gameData, Fighter *fighter = nullptr) override {
    (void)fighter;
    Fighter *who = gameData.self;
    if (!who) {
      return false;
    }
    std::string name = who->getName();

    std::transform(name.begin(), name.end(), name.begin(), [](char c) {
      return (char)std::tolower(c);
    }); //  make it lowercase

    return name == "sherlockholms" || name == "watson";
  }
};
