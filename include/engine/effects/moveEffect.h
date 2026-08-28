#pragma once
#include "effect.h"
#include "model/fighter.h"
#include "model/map.h"
#include "model/tile.h"
#include "model/typeOfFighter.h"
#include <algorithm>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

class MoveEffect : public Effect {
private:
  int howMany;
  std::string whichOne;
  int distance;
  std::string toWhere;
 std::string performer;

 static std::function<void(std::vector<Tile *>, std::function<void(Tile *)>)>
  makeTilePicker(gameData &gameData, const std::string &performer) {
    if (performer == "enemy" && gameData.requestTileChoiceFor) {
      auto requestFor = gameData.requestTileChoiceFor;
      Fighter *chooser = gameData.enemy;
      return [requestFor, chooser](std::vector<Tile *> options,
                                    std::function<void(Tile *)> onChosen) {
        requestFor(chooser, std::move(options), std::move(onChosen));
      };
    }
    auto requestTileChoice = gameData.requestTileChoice;
    return [requestTileChoice](std::vector<Tile *> options,
                                std::function<void(Tile *)> onChosen) {
      requestTileChoice(std::move(options), std::move(onChosen));
    };
  }

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

std::vector<Tile *> filterByToWhere(Map *map, Fighter *mover, gameData *data,
                                       std::vector<Tile *> candidates) const {
    if (toWhere == "nearHero") {
      if (!mover) {
        return candidates;
      }
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
      if (!mover) {
        return candidates;
      }
      keepTilesWhere(candidates, [&](Tile *t) {
        return isAdjacentToEnemyFighter(map, mover, t);
      });
      return candidates;
    }

    if (toWhere == "space_with_fog_token") {
      keepTilesWhere(candidates, [&](Tile *t) {
        return map->hasFogToken(t->getId());
      });
      return candidates;
    }

    if (toWhere == "space_without_fighters") {
      keepTilesWhere(candidates, [&](Tile *t) {
        return !map->isOccupied(t->getId());
      });
      return candidates;
    }

    if (toWhere == "same_space_as_moved_fog_token") {
      if (!data || data->lastMovedFogTokenTile < 0) {
        return {};
      }
      keepTilesWhere(candidates, [&](Tile *t) {
        return t->getId() == data->lastMovedFogTokenTile;
      });
      return candidates;
    }

    // "any", "none", and anything else: no filtering.
    return candidates;
  }

std::vector<Tile *> getFogReachableTiles(Map *map, int fromTileId, int distance) const {
    std::vector<Tile *> result;
    if (!map->getTile(fromTileId)) {
      return result;
    }

    std::unordered_map<int, int> depth;
    std::queue<int> que;
    depth[fromTileId] = 0;
    que.push(fromTileId);

    while (!que.empty()) {
      int current = que.front();
      que.pop();
      int currentDepth = depth[current];
      if (currentDepth >= distance) {
        continue;
      }

      Tile *tile = map->getTile(current);
      if (!tile) {
        continue;
      }

      for (int neighborId : tile->getNeighbors()) {
        if (depth.find(neighborId) != depth.end()) {
          continue;
        }
        depth[neighborId] = currentDepth + 1;
        que.push(neighborId);
      }
    }

    for (auto &[tileId, d] : depth) {
      (void)d;
      if (tileId == fromTileId) {
        continue;
      }
      if (Tile *t = map->getTile(tileId)) {
        result.push_back(t);
      }
    }
    return result;
  }

void executeFogTokenMove(gameData &gameData, std::function<void()> onDone) const {
    Map *map = gameData.map;
    auto requestTileChoice = makeTilePicker(gameData, performer);
    int howManyTokens = howMany > 0 ? howMany : 1;
    int range = distance >= 0 ? distance : 99;
    struct gameData *dataPtr = &gameData;

    auto remaining = std::make_shared<int>(howManyTokens);
    auto onDoneShared = std::make_shared<std::function<void()>>(std::move(onDone));
    auto stepPtr = std::make_shared<std::function<void()>>();

    *stepPtr = [this, map, requestTileChoice, range, remaining, stepPtr, onDoneShared, dataPtr]() {
      if (*remaining <= 0) {
        if (*onDoneShared) {
          (*onDoneShared)();
        }
        return;
      }

      std::vector<Tile *> sourceCandidates;
      for (int tileId : map->getFogTokenTileIds()) {
        if (Tile *t = map->getTile(tileId)) {
          sourceCandidates.push_back(t);
        }
      }
      if (sourceCandidates.empty()) {
        if (*onDoneShared) {
          (*onDoneShared)();
        }
        return;
      }

      requestTileChoice(sourceCandidates, [this, map, requestTileChoice, range, remaining,
                                            stepPtr, onDoneShared, dataPtr](Tile *source) {
        if (!source) {
          --*remaining;
          (*stepPtr)();
          return;
        }

        int fromTileId = source->getId();
        auto destCandidates =
            filterByToWhere(map, nullptr, dataPtr, getFogReachableTiles(map, fromTileId, range));

       
        destCandidates.erase(
            std::remove_if(destCandidates.begin(), destCandidates.end(),
                            [map](Tile *t) { return map->hasFogToken(t->getId()); }),
            destCandidates.end());

        if (destCandidates.empty()) {
          --*remaining;
          (*stepPtr)();
          return;
        }

        requestTileChoice(destCandidates, [map, fromTileId, remaining, stepPtr, dataPtr](Tile *dest) {
          if (dest && dest->getId() != fromTileId) {
            map->moveFogToken(fromTileId, dest->getId());
            dataPtr->lastMovedFogTokenTile = dest->getId();
          }
          --*remaining;
          (*stepPtr)();
        });
      });
    };
    (*stepPtr)();
  }

  bool allowStay;

public:

//constructor
  MoveEffect(int howMany = 1, std::string whichOne = "self", int distance = -1,
             std::string toWhere = "none", bool allowStay = false,
             std::string performer = "self")
      : howMany(howMany), whichOne(std::move(whichOne)), distance(distance),
        toWhere(std::move(toWhere)), performer(std::move(performer)),
        allowStay(allowStay) {}
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

    if (whichOne == "fog_token") {
      executeFogTokenMove(gameData, std::move(onDone));
      return;
    }

    auto movers = std::make_shared<std::vector<Fighter *>>(resolveMovers(gameData));
    auto index = std::make_shared<size_t>(0);
    Map *map = gameData.map;
    auto requestTileChoice = makeTilePicker(gameData, performer);
    int distance = this->distance;
    bool allowStay = this->allowStay;
    struct gameData *dataPtr = &gameData;
    auto onDoneShared = std::make_shared<std::function<void()>>(std::move(onDone)); // effect what should be done in the end

    auto stepPtr = std::make_shared<std::function<void()>>();
    *stepPtr = [this, movers, index, map, requestTileChoice, distance, allowStay, stepPtr, onDoneShared, dataPtr]() {
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
            ? filterByToWhere(map, mover, dataPtr, map->getReachableTiles(fromTileId, range, mover))
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
