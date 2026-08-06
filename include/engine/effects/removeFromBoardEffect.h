#pragma once
#include "effect.h"
#include "model/fighter.h"
#include "model/map.h"
#include <string>
// we use this effect for hiding enemies and 
// removing them from map
class RemoveFromBoardEffect : public Effect {
private:
  std::string whichOne;

public:
  explicit RemoveFromBoardEffect(std::string whichOne = "self")
      : whichOne(std::move(whichOne)) {}
  ~RemoveFromBoardEffect() override = default;

  void executeImmediate(gameData &gameData) override {
    if (!conditionsMet(gameData)) {
      return;
    }
    if (!gameData.map) {
      return;
    }

    Fighter *who = (whichOne == "target") ? gameData.target
                   : (whichOne == "enemy") ? gameData.enemy
                                           : gameData.self;
    if (!who) {
      return;
    }

    gameData.map->removeFighter(who);
  }
};
