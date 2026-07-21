#pragma once
#include "condition.h"
#include "model/fighter.h"


class IsLossedCondition : public Condition {
public:
  IsLossedCondition() = default;
  ~IsLossedCondition() override = default;

  bool check(gameData &gameData, Fighter *fighter = nullptr) override {
    Fighter *who = fighter ? fighter : gameData.self;
    return who && gameData.lastCombatLoser && who == gameData.lastCombatLoser;
  }
};
