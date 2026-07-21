#include "model/hero.h"
#include "model/sidekick.h"
#include "model/deck.h"

Hero::Hero() = default;
Hero::~Hero() = default;

const std::vector<std::unique_ptr<Sidekick>> &Hero::getSidekicks() const {
  return sidekicks;
}

void Hero::addSidekick(std::unique_ptr<Sidekick> sidekick) {
  sidekicks.push_back(std::move(sidekick));
}

Deck *Hero::getDeck() const { return deck.get(); }
void Hero::setDeck(std::unique_ptr<Deck> newDeck) { deck = std::move(newDeck); }
