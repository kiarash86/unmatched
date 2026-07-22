#pragma once
#include "effect.h"
#include "model/card.h"

class PredictEffect : public Effect {
public:
  PredictEffect() = default;
  ~PredictEffect() override = default;

  bool requiresPrediction() const override { return true; }

  void executeImmediate(gameData &gameData) override {
    if (!conditionsMet(gameData)) {
      return;
    }

    Card *ownCard = gameData.cardPlayed;
    Card *enemyCard = gameData.enemyCardPlayed;
    if (!ownCard || !enemyCard) {
      return;
    }
    if (ownCard->getPredictedValue() < 0) {
      return;
    }
    if (ownCard->getPredictedValue() == enemyCard->getAttackStat()) { // our predict was ok?
      enemyCard->cancelEffects(-1);
      enemyCard->setValue(0);
    }
  }
};
