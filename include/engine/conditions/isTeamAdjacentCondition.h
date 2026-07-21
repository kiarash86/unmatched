#pragma once
#include "condition.h"
#include "model/fighter.h"
#include "model/map.h"
#include "model/typeOfFighter.h"


class IsTeamAdjacentCondition : public Condition {
private:
  int distance{1};

public:
  explicit IsTeamAdjacentCondition(int distance = 1) : distance(distance) {}
  ~IsTeamAdjacentCondition() override = default;

  bool check(gameData &gameData, Fighter *fighter = nullptr) override {
    Fighter *who = fighter ? fighter : gameData.self;
    if (!who || !gameData.map) return false;

    int player = who->getOwnerPlayer();
    auto heroes = gameData.map->getFighter(TypeOfFighter::hero, player);
    auto sidekicks = gameData.map->getFighter(TypeOfFighter::sidekick, player);
    if (heroes.empty() || sidekicks.empty()) return false;

    for (auto *hero : heroes) {
      int heroTile = gameData.map->getTileIdOf(hero);
      for (auto *sk : sidekicks) {
        int skTile = gameData.map->getTileIdOf(sk);
        int d = gameData.map->distanceBetween(heroTile, skTile);
        if (d >= 0 && d <= distance) return true;
      }
    }
    return false;
  }
};
