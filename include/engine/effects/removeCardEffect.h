#pragma once
#include "effect.h"
#include "model/fighter.h"
#include "model/hero.h"
#include "model/deck.h"
#include "model/card.h"

class RemoveCardEffect : public Effect {
private:
  int howMany{1};

public:
  explicit RemoveCardEffect(int howMany = 1) : howMany(howMany) {}
  ~RemoveCardEffect() override = default;

  void execute(gameData &gameData) override {
    if (!conditionsMet(gameData)) return;
    if (!gameData.self) return;

    auto *hero = dynamic_cast<Hero *>(gameData.self);
    if (!hero || !hero->getDeck()) return;

    int finalHowMany = howMany + sumQueries(gameData);
    auto hand = hero->getDeck()->getHand();
    for (int i = 0; i < finalHowMany && i < (int)hand.size(); i++) {
      hero->getDeck()->discard(hand[i]);
    }
  }
};
