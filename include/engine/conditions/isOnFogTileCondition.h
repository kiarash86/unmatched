#pragma once
#include "condition.h"
#include "model/fighter.h"
#include "model/map.h"

class IsOnFogTileCondition : public Condition {
private:
  bool checkTurnStart;

public:
  explicit IsOnFogTileCondition(bool checkTurnStart = false)
      : checkTurnStart(checkTurnStart) {}
  ~IsOnFogTileCondition() override = default;

  bool check(gameData &gameData, Fighter *fighter = nullptr) override {
    if (checkTurnStart) {
      return gameData.selfStartedTurnOnFogTile;
    }
    Fighter *who = fighter ? fighter : gameData.self;
    if (!who || !gameData.map) {
      return false;
    }
    return gameData.map->hasFogToken(gameData.map->getTileIdOf(who));
  }
};
