#pragma once
#include "effect.h"
#include "model/fighter.h"
#include "model/map.h"
#include "model/tile.h"
#include <algorithm>
#include <string>


class ChoosePlaceEffect : public Effect {
private:
  std::string who;

public:
  explicit ChoosePlaceEffect(std::string who = "self") : who(std::move(who)) {}
  ~ChoosePlaceEffect() override = default;

  void execute(gameData &gameData) override {
    if (!conditionsMet(gameData)) return;

    Fighter *mover = (who == "target") ? gameData.target : gameData.self;
    if (!mover || !gameData.map || !gameData.requestTileChoice) return;

    int fromTile = gameData.map->getTileIdOf(mover);
    Map *map = gameData.map;

    std::vector<Tile *> options;
    for (auto &[id, tile] : map->getTiles()) {
      if (id == fromTile) continue;
      if (map->isOccupied(id)) continue;
      options.push_back(tile.get());
    }

    gameData.requestTileChoice(options, [map, mover](Tile *chosen) {
      if (chosen) map->placeFighter(mover, chosen->getId());
    });
  }
};
