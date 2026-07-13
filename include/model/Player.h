#pragma once
#include "model/heroList.h"
#include <string>
class Player // player
{
private:
  int playerIndex = 0;
  HeroList heroId = HeroList::Dracula;
  std::string heroName;

public:
  Player() = default;

  Player(int playerIndex, HeroList heroId, const std::string &heroName)
      : playerIndex(playerIndex), heroId(heroId), heroName(heroName) {}

  ~Player() = default;

  int getPlayerIndex() const { return playerIndex; }
  HeroList getHeroId() const { return heroId; }
  std::string getHeroName() const { return heroName; }
};