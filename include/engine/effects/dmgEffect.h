#pragma once
#include "effect.h"
#include "model/fighter.h"
#include "model/card.h"

class DmgEffect : public Effect {
private:
  int value{};

public:
  explicit DmgEffect(int value = 0) : value(value) {}
  ~DmgEffect() override = default;

  bool needsTarget() const override { return true; }

  void execute(gameData &gameData) override {
    if (!conditionsMet(gameData)) return;
    if (!gameData.target) return;

    int finalValue = value + sumQueries(gameData);

    gameData.target->damage(finalValue);
  }
};
//adding value to value