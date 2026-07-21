#pragma once
#include "effect.h"
#include "model/fighter.h"
#include "model/card.h"
#include "model/map.h"
#include "model/typeOfFighter.h"
#include <string>
#include <vector>


class AddEffect : public Effect {
private:
  std::string toWhat;
  std::string toWho;
  int value{};

  std::vector<Fighter *> resolveTargets(gameData &gameData) const {
    if (toWho == "enemy") return {gameData.enemy};
    if (toWho == "target") return {gameData.target};
    if (toWho == "all") {
     
      std::vector<Fighter *> team;
      if (gameData.self && gameData.map) {
        int player = gameData.self->getOwnerPlayer();
        for (auto *f : gameData.map->getFighter(TypeOfFighter::hero, player)) {
          team.push_back(f);
        }
        for (auto *f : gameData.map->getFighter(TypeOfFighter::sidekick, player)) {
          team.push_back(f);
        }
      }
      return team;
    }
    return {gameData.self}; 
  }

public:
  AddEffect(std::string toWhat, std::string toWho, int value)
      : toWhat(std::move(toWhat)), toWho(std::move(toWho)), value(value) {}
  ~AddEffect() override = default;

  bool needsTarget() const override { return toWho == "target"; }

  void execute(gameData &gameData) override {
    if (!conditionsMet(gameData)) return;

    int finalValue = value + sumQueries(gameData);

    if (toWhat == "health") {
      for (auto *fighter : resolveTargets(gameData)) {
        if (fighter) fighter->heal(finalValue);
      }
      return;
    }
    if (toWhat == "attack" || toWhat == "value") {
      if (gameData.cardPlayed) gameData.cardPlayed->modifyValue(finalValue);
      return;
    }
    if (toWhat == "actions" || toWhat == "action") {
      if (gameData.grantAction) gameData.grantAction(finalValue);
      return;
    }

  }
};
