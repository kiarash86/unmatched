#include "../../include/factory/deckFactory.h"

std::vector<std::unique_ptr<Card>> create(const std::string pathFolder) {

  std::vector<std::unique_ptr<Card>> deck;
  for (const auto &crd : listFiles(pathFolder)) {
    deck.push_back(CardFactory::create(load(crd)));
  }
  return deck;
}
