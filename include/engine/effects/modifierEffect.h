#pragma once
#include "effect.h"
#include "model/card.h"

class ModifierEffect : public Effect {
private:
  int value{};

public:
  explicit ModifierEffect(int value) : value(value) {}
  ~ModifierEffect() override = default;

  void execute(gameData &gameData) override {
    if (!conditionsMet(gameData)) return;
    if (!gameData.cardPlayed) return;

    int finalValue = value + sumQueries(gameData);
    gameData.cardPlayed->modifyValue(finalValue);
  }
};
