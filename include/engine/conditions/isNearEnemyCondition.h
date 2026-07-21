#pragma once
#include "condition.h"
#include "model/fighter.h"
#include "model/map.h"

class IsNearEnemyCondition : public Condition {
private:
  int distance{1};

public:
  explicit IsNearEnemyCondition(int distance = 1) : distance(distance) {}
  ~IsNearEnemyCondition() override = default;

  bool check(gameData &gameData, Fighter *fighter = nullptr) override {
    Fighter *who = fighter ? fighter : gameData.self;
    if (!who || !gameData.enemy || !gameData.map) {
      return false;
    }
    int fromTile = gameData.map->getTileIdOf(who);
    int toTile = gameData.map->getTileIdOf(gameData.enemy);
    int d = gameData.map->distanceBetween(fromTile, toTile);
    return d >= 0 && d <= distance;
  }
};
