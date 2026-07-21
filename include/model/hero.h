#pragma once
#include "fighter.h"
#include <memory>
#include <vector>

class Deck;
class Sidekick;

class Hero : public Fighter {
private:
  std::vector<std::unique_ptr<Sidekick>> sidekicks;
  std::unique_ptr<Deck> deck;

public:
  const std::vector<std::unique_ptr<Sidekick>> &getSidekicks() const;
  void addSidekick(std::unique_ptr<Sidekick> sidekick);

  Deck *getDeck() const;
  void setDeck(std::unique_ptr<Deck> newDeck);

  Hero();
  ~Hero() override;

  TypeOfFighter getFighterType() const override { return TypeOfFighter::hero; }
};
