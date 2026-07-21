#pragma once
#include "effect.h"
#include "model/fighter.h"
#include "model/map.h"
#include "model/tile.h"
#include "model/typeOfFighter.h"
#include <algorithm>
#include <string>
#include <vector>

class MoveEffect : public Effect {
private:
  int howMany;       
  std::string whichOne;
  int distance;       
  std::string toWhere; 

  std::vector<Fighter *> resolveMovers(gameData &gameData) const {
    if (whichOne == "self") return {gameData.self};
    if (whichOne == "enemy") return {gameData.enemy};
    if (whichOne == "hero") {
      if (!gameData.self || !gameData.map) return {};
      auto heroes = gameData.map->getFighter(TypeOfFighter::hero,
                                              gameData.self->getOwnerPlayer());
      if (heroes.empty()) return {};
      return {heroes.front()};
    }
    if (whichOne == "target") return {gameData.target};
    if (whichOne == "both") return {gameData.self, gameData.enemy};

    if (whichOne == "sidekick" || whichOne == "all") {
      if (!gameData.self || !gameData.map) return {};
      auto sidekicks = gameData.map->getFighter(TypeOfFighter::sidekick,
                                                  gameData.self->getOwnerPlayer());
      if (whichOne == "sidekick") {
        int cap = howMany > 0 ? howMany : 1;
        if ((int)sidekicks.size() > cap) sidekicks.resize(cap);
      }
      return sidekicks;
    }

    return {gameData.self}; 
  }


  std::vector<Tile *> filterByToWhere(Map *map, Fighter *mover,
                                       std::vector<Tile *> candidates) const {
    if (toWhere == "nearHero") {
      auto owners = map->getFighter(TypeOfFighter::hero, mover->getOwnerPlayer());
      if (owners.empty()) return {};
      int heroTile = map->getTileIdOf(owners.front());
      candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
                                       [&](Tile *t) {
                                         return map->distanceBetween(t->getId(), heroTile) != 1;
                                       }),
                        candidates.end());
      return candidates;
    }

    if (toWhere == "near_enemy_fighters") {
      candidates.erase(
          std::remove_if(candidates.begin(), candidates.end(),
                          [&](Tile *t) {
                            for (auto &[id, tile] : map->getTiles()) {
                              (void)id;
                              Fighter *f = map->getFighterAt(tile->getId());
                              if (!f || f->getOwnerPlayer() == mover->getOwnerPlayer()) continue;
                              if (map->distanceBetween(t->getId(), tile->getId()) == 1) return false;
                            }
                            return true;
                          }),
          candidates.end());
      return candidates;
    }

    return candidates; 
  }

public:
  MoveEffect(int howMany = 1, std::string whichOne = "self", int distance = -1,
             std::string toWhere = "none")
      : howMany(howMany), whichOne(std::move(whichOne)), distance(distance),
        toWhere(std::move(toWhere)) {}
  ~MoveEffect() override = default;

  void execute(gameData &gameData) override {
    if (!conditionsMet(gameData)) return;
    if (!gameData.map || !gameData.requestTileChoice) return;

  
    auto movers = std::make_shared<std::vector<Fighter *>>(resolveMovers(gameData));
    auto index = std::make_shared<size_t>(0);
    Map *map = gameData.map;
    auto requestTileChoice = gameData.requestTileChoice;
    int distance = this->distance;

    auto stepPtr = std::make_shared<std::function<void()>>();
    *stepPtr = [this, movers, index, map, requestTileChoice, distance, stepPtr]() {
      while (*index < movers->size()) {
        Fighter *mover = (*movers)[*index];
        ++*index;
        if (!mover || !mover->isAlive()) continue;

        int fromTileId = map->getTileIdOf(mover);
        int range = distance >= 0 ? distance : mover->getMovement();
        if (range <= 0) continue;

        auto candidates = filterByToWhere(map, mover, map->getReachableTiles(fromTileId, range, mover));
        if (candidates.empty()) continue;

      
        requestTileChoice(candidates, [map, mover, stepPtr](Tile *chosen) {
          if (chosen) map->placeFighter(mover, chosen->getId());
          (*stepPtr)();
        });
        return; 
      }
    };
    (*stepPtr)();
  }
};
