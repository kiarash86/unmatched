#pragma once
#include "effect.h"
#include "model/fighter.h"
#include "model/map.h"
#include "model/tile.h"
#include <algorithm>
#include <string>
// it doesnt have the ability to choose place later
// so we use a string conditon that give us the time of 
// choosing it
class ChoosePlaceEffect : public Effect {
private:
  std::string who;

  std::string when;


  void placeNow(gameData &gameData, std::function<void()> onDone) {
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
                     Tile *chosen) { 
          if (chosen) {
            map->placeFighter(mover, chosen->getId());
          }
          if (onDone) {

            onDone();
          }
        });
  }

public:
  explicit ChoosePlaceEffect(std::string who = "self",
                              std::string when = "immediate")
      : who(std::move(who)), when(std::move(when)) {} // main func of placeEffect

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
    if (!mover) {
      if (onDone) {
        onDone();
      }
      return;
    }

    if (when == "start_of_next_turn" && gameData.deferToStartOfNextTurn) {
    
      gameData.deferToStartOfNextTurn(mover, this);
      if (onDone) {
        onDone();
      }
      return;
    }

   
    placeNow(gameData, std::move(onDone));
  }

  void executeDeferred(gameData &gameData, std::function<void()> onDone) override {
    placeNow(gameData, std::move(onDone));
  }
};
