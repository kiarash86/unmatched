#pragma once
#include "engine/gameData.h"
#include "model/card.h"
#include "query.h"


class CardBoost : public Query {
private:
  bool useEnemy{false};

public:
  explicit CardBoost(bool useEnemy = false) : useEnemy(useEnemy) {}

  int get(gameData &gameData) override {
    Card *card = useEnemy ? gameData.enemyCardPlayed : gameData.cardPlayed;
    if (!card) return 0;
    for (auto &&cnd : conditions) {
      if (!cnd->check(gameData)) return 0;
    }
    return card->getBoost();
  }
};
