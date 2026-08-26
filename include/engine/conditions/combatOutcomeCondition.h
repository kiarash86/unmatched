#pragma once
#include "condition.h"
#include "model/fighter.h"

enum class CombatOutcome { Won, Lost }; //in the future should make a file for it?

class CombatOutcomeCondition : public Condition {
private:
  CombatOutcome outcome;

public:
  explicit CombatOutcomeCondition(CombatOutcome outcome) : outcome(outcome) {}
  ~CombatOutcomeCondition() override = default;

  bool check(gameData &gameData, Fighter *fighter = nullptr) override {
    Fighter *who = fighter ? fighter : gameData.self;
    Fighter *who2 = (outcome == CombatOutcome::Won) ? gameData.lastCombatWinner
                                                           : gameData.lastCombatLoser;
    return who != nullptr && who == who2;
  }
};
