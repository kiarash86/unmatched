#pragma once
#include "model/typeOfEvent.h"
#include <functional>
#include <string>
#include <vector>

class Fighter;
class Card;
class Map;
class Tile;
class Hero;
class Effect;


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

 bool selfStartedTurnOnFogTile{false};
 int lastMovedFogTokenTile{-1};

 
  Card *enemyCardPlayed{nullptr}; //def

  std::function<void(std::vector<Tile *>, std::function<void(Tile *)>)> requestTileChoice;

std::function<void(Fighter *, std::vector<Tile *>, std::function<void(Tile *)>)> requestTileChoiceFor;
  std::function<void(std::vector<Tile *>, Tile *explicitStayTile, std::function<void(Tile *)>)>
      requestTileChoiceWithStay;

  std::function<void(std::vector<Card *>, std::function<void(Card *)>)> requestCardChoice;

  std::function<void(Fighter *, std::vector<Card *>, std::function<void(Card *)>)> requestCardChoiceFor;

 std::function<void(std::vector<std::string>, std::function<void(int)>)> requestEffectChoice;

std::function<void(Fighter *, std::vector<std::string>, std::function<void(int)>)> requestEffectChoiceFor;

std::function<void(Fighter *owner, Effect *effect)> deferToStartOfNextTurn;

  std::function<bool(Hero *)> disableAbility;

  std::function<void(int)> grantAction;

  std::function<Hero *(Fighter *)> getOwnerHero;

  std::function<void(Hero *, const std::vector<Card *> &)> onHandRevealed;
};
