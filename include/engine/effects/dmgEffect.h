#pragma once
#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <vector>
#include "effect.h"
#include "engine/conditions/condition.h"
#include "model/fighter.h"
#include "model/card.h"
#include "model/map.h"
#include "model/typeOfFighter.h"

class DmgEffect : public Effect {
private:
  int value{};
  std::string toWhat;

  std::unique_ptr<Condition> boostCondition;
  int boostValue{0};

  static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                    [](unsigned char c) { return std::tolower(c); });
    return s;
  }

  bool isAutoTargeted() const {
    std::string w = toLower(toWhat);
    return w == "near_hero" || w == "adjacent_fighter" ||
           w == "each_enemy_fighter" || w == "enemy_fighters_on_fog_token";
  }

  // who is your target
  std::vector<Fighter *> resolveTargets(gameData &gameData) const {
    std::string what = toLower(toWhat);

    if (what == "near_hero" || what == "adjacent_fighter") {
      std::vector<Fighter *> results;
      if (gameData.self && gameData.enemy && gameData.map) {
        int enemyPlayer = gameData.enemy->getOwnerPlayer();
        int selfTile = gameData.map->getTileIdOf(gameData.self);
        for (auto type : {TypeOfFighter::hero, TypeOfFighter::sidekick}) {
          for (auto *f : gameData.map->getFighter(type, enemyPlayer)) {
            int d = gameData.map->distanceBetween(selfTile, gameData.map->getTileIdOf(f));
            if (d >= 0 && d <= 1) {
              results.push_back(f);
            }
          }
        }
      }
      return results;
    }
 if (what == "each_enemy_fighter" || what == "enemy_fighters_on_fog_token") {
      std::vector<Fighter *> results;
      if (gameData.self && gameData.enemy && gameData.map) {
        int enemyPlayer = gameData.enemy->getOwnerPlayer();
        for (auto type : {TypeOfFighter::hero, TypeOfFighter::sidekick}) {
          for (auto *f : gameData.map->getFighter(type, enemyPlayer)) {
            if (!f || !f->isAlive()) {
              continue;
            }
            if (what == "enemy_fighters_on_fog_token" &&
                !gameData.map->hasFogToken(gameData.map->getTileIdOf(f))) {
              continue;
            }
            results.push_back(f);
          }
        }
      }
      return results;
    }

    return {gameData.target};
  }

public:
//constructor
  explicit DmgEffect(int value = 0, std::string toWhat = "")
      : value(value), toWhat(std::move(toWhat)) {}
//constructor

//destructor 
  ~DmgEffect() override = default;
//destructor 

 void setConditionalValue(std::unique_ptr<Condition> cond, int boostedValue) {
    boostCondition = std::move(cond);
    boostValue = boostedValue;
  }

// choosing target for damaging 
   bool needsTarget() const override { return !isAutoTargeted(); } // automatically dmg for the auto-targeted toWhat variants
// choosing target for damaging 

// damage immediatly, whenever effect apears with this 
//this is damage it
  void executeImmediate(gameData &gameData) override {
    if (!conditionsMet(gameData)) {
      return;
    }

    int baseValue = value;
    if (boostCondition && boostCondition->check(gameData)) {
      baseValue = boostValue;
    }
    int finalValue = baseValue + sumQueries(gameData);

    for (auto *fighter : resolveTargets(gameData)) {
      if (fighter) {
        fighter->damage(finalValue);
      }
    }
  }

// damage immediatly, whenever effect apears with this 
//this is damage it
};
