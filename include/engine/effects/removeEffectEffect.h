#pragma once
#include "effect.h"
#include "model/card.h"
#include "model/fighter.h"
#include "model/hero.h"
#include <string>

class RemoveEffectEffect : public Effect {
private:
  int howMany{-1};
  std::string whichOne{"all"};
  bool targetEnemy{true};
  bool targetsAbility{false};

public:
  RemoveEffectEffect(int howMany = -1, std::string whichOne = "all",
                     bool targetEnemy = true, bool targetsAbility = false)
      : howMany(howMany), whichOne(std::move(whichOne)),
        targetEnemy(targetEnemy), targetsAbility(targetsAbility) {}
  ~RemoveEffectEffect() override = default;

  void executeImmediate(gameData &gameData) override {
    if (!conditionsMet(gameData)) {
      return;
    }

    Fighter *who = targetEnemy ? gameData.enemy : gameData.self;
    if (!who) {
      return;
    }

    if (targetsAbility) {
      auto *hero = dynamic_cast<Hero *>(who);
      if (!hero || !gameData.disableAbility) {
        return;
      }
      gameData.disableAbility(hero); // disable it by sherlock ability
      return;
    }

    Card *targetCard =
        targetEnemy ? gameData.enemyCardPlayed : gameData.cardPlayed;
    if (!targetCard) {
      return;
    }

    int amount = (whichOne == "all" || howMany < 0) ? -1 : howMany;
    targetCard->cancelEffects(amount);
  }
};