#pragma once
#include "model/typeOfEvent.h"
#include <functional>
#include <vector>

class Fighter;
class Card;
class Map;
class Tile;


struct gameData {
  Map *map{nullptr}; //board
  Fighter *self{nullptr}; // me
  Fighter *target{nullptr};
  Fighter *enemy{nullptr};
  Card *cardPlayed{nullptr};
  int value{0};
  TypeOfEvent event{TypeOfEvent::none};


  Fighter *lastCombatWinner{nullptr}; // for exchanging position for example
  Fighter *lastCombatLoser{nullptr};

 
  Card *enemyCardPlayed{nullptr}; //def

  std::function<void(std::vector<Tile *>, std::function<void(Tile *)>)> requestTileChoice; // get tile from user but which tiles?
  // tiles in a same zone? or near fighter? or...?
};
