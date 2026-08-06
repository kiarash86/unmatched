#pragma once
#include "effect.h"
#include <memory>

// we have a basic version of it in game manager
// but dont accepting that and decline choice and
// give this chance to enemy? no it doesnt have
// so just convert it to effect and we use it 
// like the one that we have in game manager
class OpponentChoiceEffect : public Effect {
private:
  std::unique_ptr<Effect> choiceEffect;
  std::unique_ptr<Effect> declineEffect;

public:
  OpponentChoiceEffect(std::unique_ptr<Effect> choiceEffect,
                        std::unique_ptr<Effect> declineEffect)
      : choiceEffect(std::move(choiceEffect)),
        declineEffect(std::move(declineEffect)) {}
  ~OpponentChoiceEffect() override = default;

  void execute(gameData &gameData, std::function<void()> onDone) override {
    if (!conditionsMet(gameData)) {
      if (onDone) {
        onDone();
      }
      return;
    }


    if (!choiceEffect || !declineEffect) {
      if (choiceEffect) {
        choiceEffect->execute(gameData, onDone);
        return;
      }
      if (declineEffect) {
        declineEffect->execute(gameData, onDone);
        return;
      }
      if (onDone) {
        onDone();
      }
      return;
    }

    if (!gameData.enemy || !gameData.requestEffectChoiceFor) {
      choiceEffect->execute(gameData, onDone);
      return;
    }

    Fighter *decisionMaker = gameData.enemy;

    gameData.requestEffectChoiceFor(
        decisionMaker, {"Accept", "Decline"},
        [this, &gameData, onDone](int index) {
          if (index == 1) {
            declineEffect->execute(gameData, onDone);
          } else {
            choiceEffect->execute(gameData, onDone);
          }
        });
  }
};
