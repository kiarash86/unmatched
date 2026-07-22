#pragma once
#include "model/typeOfEvent.h"
#include <functional>
#include <vector>

class Fighter;
class Card;
class Map;
class Tile;
class Hero;


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

  std::function<void(std::vector<Tile *>, std::function<void(Tile *)>)> requestTileChoice;
 
  std::function<void(std::vector<Card *>, std::function<void(Card *)>)> requestCardChoice;

  std::function<bool(Hero *)> disableAbility;

  std::function<void(int)> grantAction;

  std::function<Hero *(Fighter *)> getOwnerHero;

  std::function<void(Hero *, const std::vector<Card *> &)> onHandRevealed;
};
