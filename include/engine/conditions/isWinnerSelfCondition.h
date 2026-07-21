#pragma once
#include "condition.h"
#include "model/fighter.h"
//something like islossed 
//but better to split it to 2 parts
class IsWinnerSelfCondition : public Condition {
public:
  IsWinnerSelfCondition() = default;
  ~IsWinnerSelfCondition() override = default;

  bool check(gameData &gameData, Fighter *fighter = nullptr) override {
    (void)fighter;
    return gameData.self && gameData.lastCombatWinner &&
           gameData.self == gameData.lastCombatWinner;
  }
};
