#pragma once
#include "condition.h"
#include "model/fighter.h"

enum class CombatOutcome { Won, Lost };

class CombatOutcomeCondition : public Condition {
private:
  CombatOutcome outcome;

public:
  explicit CombatOutcomeCondition(CombatOutcome outcome) : outcome(outcome) {}
  ~CombatOutcomeCondition() override = default;

  bool check(gameData &gameData, Fighter *fighter = nullptr) override {
    Fighter *who = fighter ? fighter : gameData.self;
    Fighter *reference = (outcome == CombatOutcome::Won) ? gameData.lastCombatWinner
                                                           : gameData.lastCombatLoser;
    return who && reference && who == reference;
  }
};
