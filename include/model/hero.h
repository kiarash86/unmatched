#pragma once
#include "fighter.h"
#include <memory>
#include <vector>

class Deck;
class Sidekick;
class Ability;

class Hero : public Fighter {
private:
  std::vector<std::unique_ptr<Sidekick>> sidekicks;
  std::unique_ptr<Deck> deck;
  std::unique_ptr<Ability> ability;

public:
  const std::vector<std::unique_ptr<Sidekick>> &getSidekicks() const;
  void addSidekick(std::unique_ptr<Sidekick> sidekick);

  Ability *getAbility() const;
  void setAbility(std::unique_ptr<Ability> newAbility);

  Deck *getDeck() const;
  void setDeck(std::unique_ptr<Deck> newDeck);

  Hero();
  ~Hero() override;

  TypeOfFighter getFighterType() const override { return TypeOfFighter::Hero; }
};
