#pragma once
#include "effect.h"
#include "model/card.h"


class ChangeValueEffect : public Effect {
private:
  bool targetEnemy{false};
  bool useBoost{false};
  int flatAmount{0};

public:
  ChangeValueEffect(bool targetEnemy, bool useBoost, int flatAmount = 0)
      : targetEnemy(targetEnemy), useBoost(useBoost), flatAmount(flatAmount) {}
  ~ChangeValueEffect() override = default;

  void executeImmediate(gameData &gameData) override {
    if (!conditionsMet(gameData)) return;

    Card *targetCard = targetEnemy ? gameData.enemyCardPlayed : gameData.cardPlayed;
    if (!targetCard) return;

    if (useBoost) { // for reversing boost and value
      targetCard->setValue(targetCard->getBoost());
      return;
    }

    int amount = flatAmount + sumQueries(gameData);
    targetCard->modifyValue(amount);
  }
};
