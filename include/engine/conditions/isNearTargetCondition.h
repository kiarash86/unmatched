#pragma once
#include "condition.h"
#include "model/fighter.h"
#include "model/map.h"


class IsNearTargetCondition : public Condition {
private:
  int distance{1};

public:
  explicit IsNearTargetCondition(int distance = 1) : distance(distance) {}
  ~IsNearTargetCondition() override = default;

  bool check(gameData &gameData, Fighter *fighter = nullptr) override {
    Fighter *who = fighter ? fighter : gameData.self;
    if (!who || !gameData.target || !gameData.map) {
      return false;
    }
    int fromTile = gameData.map->getTileIdOf(who);
    int toTile = gameData.map->getTileIdOf(gameData.target);
    int d = gameData.map->distanceBetween(fromTile, toTile);
    return d >= 0 && d <= distance;
  }
};
