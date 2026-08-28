#pragma once
#include "effect.h"
#include "model/card.h"
// we have a effect for modifying value
// and this is a similiar effect but 
// instead of increading or decreasing
// it change it to a fixed value 
class SetValueEffect : public Effect {
private:
  bool targetEnemy{false};
  int fixedAmount{0};
  bool unmodifiable{false};

public:
  SetValueEffect(bool targetEnemy, int fixedAmount, bool unmodifiable = false)
      : targetEnemy(targetEnemy), fixedAmount(fixedAmount), unmodifiable(unmodifiable) {}
  ~SetValueEffect() override = default;

  void executeImmediate(gameData &gameData) override {
    if (!conditionsMet(gameData)) return;

    Card *targetCard = targetEnemy ? gameData.enemyCardPlayed : gameData.cardPlayed;
    if (!targetCard) return;

    targetCard->setValue(fixedAmount);

    if (unmodifiable) {
      targetCard->lockValue();
    }
  }
};
