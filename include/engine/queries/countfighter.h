#pragma once
#include "engine/conditions/condition.h"
#include "engine/gameData.h"
#include "model/typeOfFighter.h"
#include "model/fighter.h"
#include "model/map.h"
#include "query.h"


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
      bool flag = true;
      for (auto &&cnd : conditions) {
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
