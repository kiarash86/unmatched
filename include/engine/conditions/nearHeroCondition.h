#pragma once
#include "condition.h"
#include "model/fighter.h"
#include "model/map.h"


class NearHeroCondition : public Condition {
private:
  int distance{1};

public:
  explicit NearHeroCondition(int distance = 1) : distance(distance) {}
  ~NearHeroCondition() override = default;

  bool check(gameData &gameData, Fighter *fighter = nullptr) override {
    Fighter *who = fighter ? fighter : gameData.self;
    if (!who || !gameData.self || !gameData.map) {
      return false;
    }
    int fromTile = gameData.map->getTileIdOf(who);
    int toTile = gameData.map->getTileIdOf(gameData.self);
    int d = gameData.map->distanceBetween(fromTile, toTile);
    return d >= 0 && d <= distance;
  }
};
