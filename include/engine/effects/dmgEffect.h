#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include "effect.h"
#include "model/fighter.h"
#include "model/card.h"
#include "model/map.h"
#include "model/typeOfFighter.h"

class DmgEffect : public Effect {
private:
  int value{};
  std::string toWhat;

  static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                    [](unsigned char c) { return std::tolower(c); });
    return s;
  }
  // who is your target
  std::vector<Fighter *> resolveTargets(gameData &gameData) const {
    if (toLower(toWhat) == "near_hero") { // if this is near hero, then automatically choose target
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
    //if not near hero then just target the target in gameDagta
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

// choosing target for damaging 
   bool needsTarget() const override { return toLower(toWhat) != "near_hero"; } // automatically dmg when condition is near hero 
// choosing target for damaging 

// damage immediatly, whenever effect apears with this 
//this is damage it
  void executeImmediate(gameData &gameData) override {
    if (!conditionsMet(gameData)) return;

    int finalValue = value + sumQueries(gameData);

    for (auto *fighter : resolveTargets(gameData)) {
      if (fighter) fighter->damage(finalValue);
    }
  }

// damage immediatly, whenever effect apears with this 
//this is damage it
};
