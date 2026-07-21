#pragma once
#include "effect.h"
#include "model/card.h"
//adding to def value

class DefEffect : public Effect {
private:
  int value{};

public:
  explicit DefEffect(int value = 0) : value(value) {}
  ~DefEffect() override = default;

  void execute(gameData &gameData) override {
    if (!conditionsMet(gameData)) return;
    if (!gameData.cardPlayed) return;

    int finalValue = value + sumQueries(gameData);
    gameData.cardPlayed->modifyValue(finalValue);
  }
};
