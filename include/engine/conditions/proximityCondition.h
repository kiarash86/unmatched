#pragma once
#include "condition.h"
#include "model/fighter.h"
#include "model/map.h"


class ProximityCondition : public Condition {
private:
  Fighter *gameData::*other;
  int distance{1};

public:
  explicit ProximityCondition(Fighter *gameData::*other, int distance = 1)
      : other(other), distance(distance) {}
  ~ProximityCondition() override = default;

  bool check(gameData &gameData, Fighter *fighter = nullptr) override {
    Fighter *who = fighter ? fighter : gameData.self;
    Fighter *target = gameData.*other;
    if (!who || !target || !gameData.map) {
      return false;
    }
    int fromTile = gameData.map->getTileIdOf(who);
    int toTile = gameData.map->getTileIdOf(target);
    int d = gameData.map->distanceBetween(fromTile, toTile);
    return d >= 0 && d <= distance;
  }
};
