#pragma once
#include "effect.h"
#include "model/fighter.h"
#include "model/map.h"

// Swaps gameData.self and gameData.target
class PositionExchangeEffect : public Effect {
public:
  PositionExchangeEffect() = default;
  ~PositionExchangeEffect() override = default;

  bool needsTarget() const override { return true; }

  void executeImmediate(gameData &gameData) override {
    if (!conditionsMet(gameData)) {
      return;
    }
    if (!gameData.self || !gameData.target || !gameData.map) {
      return;
    }
    gameData.map->exchangePosition(gameData.self, gameData.target);
  }
};
