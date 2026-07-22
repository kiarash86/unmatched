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
  explicit ChoosePlaceEffect(std::string who = "self")
      : who(std::move(who)) {} // main func of placeEffect

  ~ChoosePlaceEffect() override = default;

  void
  execute(gameData &gameData,
          std::function<void()> onDone) override { // the main func of excuting
    if (!conditionsMet(gameData)) {
      if (onDone) {
        onDone();
      }
      return;
    }

    Fighter *mover = (who == "target") ? gameData.target : gameData.self;
    if (!mover || !gameData.map || !gameData.requestTileChoice) {
      if (onDone) {
        onDone();
      }
      return;
    }

    int fromTile = gameData.map->getTileIdOf(mover);

    Map *map = gameData.map;

    std::vector<Tile *> options;
    for (auto &[id, tile] : map->getTiles()) {

      if (id == fromTile) {
        continue;
      }
      if (map->isOccupied(id)) {
        continue;
      }

      options.push_back(tile.get());
    }
    if (options.empty()) {
      if (onDone) {

        onDone();
      }
      return;
    } // ok nothing to do - bye

    gameData.requestTileChoice(
        options, [map, mover, onDone](
                     Tile *chosen) { // get a tile in options and who will be
                                     // this lucky person to move , when?
          if (chosen) {
            map->placeFighter(mover, chosen->getId());
          }
          if (onDone) {

            onDone();
          }
        });
  }
};
