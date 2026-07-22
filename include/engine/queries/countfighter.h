#pragma once
#include "engine/conditions/condition.h"
#include "engine/gameData.h"
#include "model/typeOfFighter.h"
#include "model/fighter.h"
#include "model/map.h"
#include "query.h"
// we have some type that we can use this
// just hero and need to find enemy's hero?
// all heroes and sidekciks and need to find enemy's?
// just sidekicks...
// and same with using enemy heroes and sidekicks

class CountFighter : public Query {
private:
  TypeOfFighter fighterType;
  bool useEnemy{false};

public:
  explicit CountFighter(TypeOfFighter type, bool useEnemy = false)
      : fighterType(type), useEnemy(useEnemy) {}

  int get(gameData &gameData) override {
    int value{0};
    if (!gameData.map || !gameData.self) return value;

    Fighter *owner = useEnemy ? gameData.enemy : gameData.self;
    if (!owner) return value;

    for (auto &&fghr : gameData.map->getFighter(fighterType, owner->getOwnerPlayer())) {
      if (fghr == gameData.target) { // we use this because if ravening if 
        //sister moves herself or others sister it would made them dead
        // because they were a neighbor with 0 distance
        continue;
      }
      bool flag = true;
      for (auto &&cnd : conditions) { //checking conditions
        if (!cnd->check(gameData, fghr)) {
          flag = false;
          break;
        }
      }
      if (flag) {
        value++;
      }
    }
    return value;
  }
};
