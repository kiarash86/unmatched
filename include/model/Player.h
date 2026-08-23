#pragma once
#include "model/heroList.h"
class Player // player
{
private:
  HeroList heroId = HeroList::Dracula;

public:
  Player() = default;

Player(HeroList heroId) : heroId(heroId) {}

  ~Player() = default;

  HeroList getHeroId() const { return heroId; }
};