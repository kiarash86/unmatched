#pragma once
#include "engine/gameData.h"
#include "model/deck.h"
#include "model/fighter.h"
#include "model/hero.h"
#include "query.h"


class CountRemovedCards : public Query {
private:
  bool useEnemy{false};

public:
  explicit CountRemovedCards(bool useEnemy = false) : useEnemy(useEnemy) {}

  int get(gameData &gameData) override {
    Fighter *who = useEnemy ? gameData.enemy : gameData.self;
    auto *hero = dynamic_cast<Hero *>(who);
    if (!hero || !hero->getDeck()) return 0;

    for (auto &&cnd : conditions) {
      if (!cnd->check(gameData)) return 0;
    }
    return hero->getDeck()->discardPileCount();
  }
};
