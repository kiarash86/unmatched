#pragma once
#include "effect.h"
#include "model/fighter.h"
#include "model/hero.h"
#include "model/sidekick.h"
#include "model/map.h"
#include "model/tile.h"
#include "model/typeOfFighter.h"
#include <algorithm>
#include <string>


class ReviveEffect : public Effect {
private:
  std::string whichOne;

  Fighter *findDefeatedSidekick(gameData &gameData) const {
    if (!gameData.self || !gameData.getOwnerHero) return nullptr;
    Hero *hero = gameData.getOwnerHero(gameData.self);
    if (!hero) return nullptr;

    for (auto &sk : hero->getSidekicks()) {
      if (sk && !sk->isAlive()) return sk.get();
    }
    return nullptr;
  }

public:
  explicit ReviveEffect(std::string whichOne = "sidekick")
      : whichOne(std::move(whichOne)) {}
  ~ReviveEffect() override = default;

  void execute(gameData &gameData) override {
    if (!conditionsMet(gameData)) return;
    if (!gameData.map || !gameData.self || !gameData.requestTileChoice) return;
    if (whichOne != "sidekick") return; 

    Fighter *defeated = findDefeatedSidekick(gameData);
    if (!defeated) return; 

    Map *map = gameData.map;
    int selfTileId = map->getTileIdOf(gameData.self);
    Tile *selfTile = map->getTile(selfTileId);
    if (!selfTile) return;

   
    std::vector<Tile *> candidates = map->getTilesInZones(selfTile->getZones());
    candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
                                     [&](Tile *t) { return map->isOccupied(t->getId()); }),
                      candidates.end());
    if (candidates.empty()) return;

    gameData.requestTileChoice(candidates, [map, defeated](Tile *chosen) {
      if (!chosen) return;
     
      defeated->setHealth(defeated->getMaxHealth());
      map->placeFighter(defeated, chosen->getId());
    });
  }
};
