#pragma once
#include "effect.h"
#include "model/fighter.h"
#include "model/hero.h"
#include "model/deck.h"


class DrawEffect : public Effect {
private:
  int amount{1};

public:
  explicit DrawEffect(int amount = 1) : amount(amount) {}
  ~DrawEffect() override = default;

  void execute(gameData &gameData) override {
    if (!conditionsMet(gameData)) return;
    if (!gameData.self) return;

    auto *hero = dynamic_cast<Hero *>(gameData.self);
    if (!hero || !hero->getDeck()) return;

    int finalAmount = amount + sumQueries(gameData);
    if (finalAmount > 0) {
      hero->getDeck()->draw(finalAmount);
    }
  }
};

//drawing card get from deck