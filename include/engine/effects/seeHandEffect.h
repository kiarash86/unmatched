#pragma once
#include "effect.h"
#include "model/fighter.h"
#include "model/hero.h"
#include "model/deck.h"


class SeeHandEffect : public Effect {
private:
  std::vector<Card *> lastSeenHand;

public:
  SeeHandEffect() = default;
  ~SeeHandEffect() override = default;

  void execute(gameData &gameData) override {
    if (!conditionsMet(gameData)) return;
    if (!gameData.enemy) return;

    auto *enemyHero = dynamic_cast<Hero *>(gameData.enemy);
    if (!enemyHero || !enemyHero->getDeck()) return;

    lastSeenHand = enemyHero->getDeck()->getHand();
  }

  const std::vector<Card *> &getSeenHand() const { return lastSeenHand; }
};
