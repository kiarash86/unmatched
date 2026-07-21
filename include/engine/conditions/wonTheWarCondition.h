#pragma once
#include "condition.h"
#include "model/fighter.h"


class WonTheWarCondition : public Condition {
private:
  int distance{1}; // i think i should delete it

public:
  explicit WonTheWarCondition(int distance = 1) : distance(distance) {}
  ~WonTheWarCondition() override = default;

  bool check(gameData &gameData, Fighter *fighter = nullptr) override {
    (void)distance;
    Fighter *who = fighter ? fighter : gameData.self;
    return who && gameData.lastCombatWinner && who == gameData.lastCombatWinner;
  }
};
