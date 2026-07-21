#pragma once
#include "condition.h"
#include "model/fighter.h"

enum class CombatOutcome { Won, Lost };

// Replaces IsLossedCondition / IsWinnerSelfCondition / WonTheWarCondition.
// All three did "is this fighter the winner/loser of the last combat?" —
// they only disagreed (inconsistently) on whether to use the passed-in
// fighter or always gameData.self. This version always uses `who`
// (fighter if given, else self), which matches IsLossedCondition and
// WonTheWarCondition's behavior; IsWinnerSelfCondition previously ignored
// the passed-in fighter, which looks like the actual bug.
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
