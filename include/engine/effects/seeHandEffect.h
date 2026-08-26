#pragma once
#include "effect.h"
#include "model/deck.h"
#include "model/fighter.h"
#include "model/hero.h"

class SeeHandEffect : public Effect {
public:
  SeeHandEffect() = default;
  ~SeeHandEffect() override = default;

  void executeImmediate(gameData &gameData) override {
    if (!conditionsMet(gameData)) {
      return;
    }
    if (!gameData.enemy) {
      return;
    }

    auto *enemyHero = gameData.getOwnerHero
                          ? gameData.getOwnerHero(gameData.enemy)
                          : dynamic_cast<Hero *>(gameData.enemy);
    if (!enemyHero || !enemyHero->getDeck()) {
      return;
    }

      std::vector<Card *> seenHand = enemyHero->getDeck()->getHand();

    if (gameData.onHandRevealed) {
      gameData.onHandRevealed(enemyHero, seenHand);
    }
  }
};