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
    if (whichOne == "self") {
      return {gameData.self};
    }
    if (whichOne == "enemy") {
      return {gameData.enemy};
    }
    if (whichOne == "hero") {
      if (!gameData.self || !gameData.map) {
        return {};
      }
      auto heroes = gameData.map->getFighter(TypeOfFighter::hero,
                                              gameData.self->getOwnerPlayer());
      if (heroes.empty()) {
        return {};
      }
      return {heroes.front()};
    }
    if (whichOne == "target") {
      return {gameData.target};
    }
    if (whichOne == "both") {
      return {gameData.self, gameData.enemy};
    }

    if (whichOne == "sidekick" || whichOne == "all") {
      if (!gameData.self || !gameData.map) {
        return {};
      }
      auto sidekicks = gameData.map->getFighter(TypeOfFighter::sidekick,
                                                  gameData.self->getOwnerPlayer());
      if (whichOne == "sidekick") {
        int cap = howMany > 0 ? howMany : 1;
        if ((int)sidekicks.size() > cap) {
          sidekicks.resize(cap);
        }
      }
      return sidekicks;
    }

    return {gameData.self};
  }


  template <typename Predicate>
  static void keepTilesWhere(std::vector<Tile *> &tiles, Predicate keep) {
    tiles.erase(std::remove_if(tiles.begin(), tiles.end(),
                                [&](Tile *t) {
                                  return !keep(t);
                                }),
                tiles.end());
  }

  bool isAdjacentToTile(Map *map, Tile *t, int otherTileId) const {
    return map->distanceBetween(t->getId(), otherTileId) == 1;
  }

  bool isAdjacentToEnemyFighter(Map *map, Fighter *mover, Tile *t) const {
    for (auto &[id, tile] : map->getTiles()) {
      (void)id;
      Fighter *occupant = map->getFighterAt(tile->getId());
      bool isEnemy = occupant && occupant->getOwnerPlayer() != mover->getOwnerPlayer();
      if (isEnemy && isAdjacentToTile(map, t, tile->getId())) {
        return true;
      }
    }
    return false;
  }

  std::vector<Tile *> filterByToWhere(Map *map, Fighter *mover,
                                       std::vector<Tile *> candidates) const {
    if (toWhere == "nearHero") {
      auto owners = map->getFighter(TypeOfFighter::hero, mover->getOwnerPlayer());
      if (owners.empty()) {
        return {};
      }

      int heroTile = map->getTileIdOf(owners.front());
      keepTilesWhere(candidates, [&](Tile *t) {
        return isAdjacentToTile(map, t, heroTile);
      });
      return candidates;
    }

    if (toWhere == "near_enemy_fighters") {
      keepTilesWhere(candidates, [&](Tile *t) {
        return isAdjacentToEnemyFighter(map, mover, t);
      });
      return candidates;
    }

    return candidates;
  }

  bool allowStay;

public:

//constructor
  MoveEffect(int howMany = 1, std::string whichOne = "self", int distance = -1,
             std::string toWhere = "none", bool allowStay = false)
      : howMany(howMany), whichOne(std::move(whichOne)), distance(distance),
        toWhere(std::move(toWhere)), allowStay(allowStay) {}
//constructor
  
//destructor
        ~MoveEffect() override = default;
//destructor


  void execute(gameData &gameData, std::function<void()> onDone) override {
    if (!conditionsMet(gameData)) {
      if (onDone) {
        onDone();
      }
      return;
    }
    if (!gameData.map || !gameData.requestTileChoice) {
      if (onDone) {
        onDone();
      }
      return;
    }

    auto movers = std::make_shared<std::vector<Fighter *>>(resolveMovers(gameData));
    auto index = std::make_shared<size_t>(0);
    Map *map = gameData.map;
    auto requestTileChoice = gameData.requestTileChoice;
    int distance = this->distance;
    bool allowStay = this->allowStay;
    auto onDoneShared = std::make_shared<std::function<void()>>(std::move(onDone)); // effect what should be done in the end

    auto stepPtr = std::make_shared<std::function<void()>>();
    *stepPtr = [this, movers, index, map, requestTileChoice, distance, allowStay, stepPtr, onDoneShared]() {
      while (*index < movers->size()) {
        Fighter *mover = (*movers)[*index];
        ++*index;
        if (!mover || !mover->isAlive()) {
          continue;
        }

        int fromTileId = map->getTileIdOf(mover);
        int range = distance >= 0 ? distance : mover->getMovement();
        if (range <= 0 && !allowStay) {
          continue;
        }

        auto candidates = range > 0
            ? filterByToWhere(map, mover, map->getReachableTiles(fromTileId, range, mover))
            : std::vector<Tile *>{};

        if (allowStay) {
  
          if (Tile *origin = map->getTile(fromTileId)) {
            candidates.push_back(origin);
          }
        }
        if (candidates.empty()) {
          continue;
        }

        requestTileChoice(candidates, [map, mover, fromTileId, stepPtr](Tile *chosen) { // we write a lambda here
          // for func

          if (chosen && chosen->getId() != fromTileId) {
            map->placeFighter(mover, chosen->getId());
          }
          (*stepPtr)();
        });
        return;
      }
    
      if (*onDoneShared) {
        (*onDoneShared)();
      }
    };
    (*stepPtr)();
  }
};