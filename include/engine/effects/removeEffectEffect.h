#pragma once
#include "effect.h"
#include "model/ability.h"
#include "model/hero.h"

class RemoveEffectEffect : public Effect {
private:
  int howMany{-1};

public:
  explicit RemoveEffectEffect(int howMany = -1) : howMany(howMany) {}
  ~RemoveEffectEffect() override = default;

  void execute(gameData &gameData) override {
    if (!conditionsMet(gameData)) return;
   
    (void)howMany;
  }
};
