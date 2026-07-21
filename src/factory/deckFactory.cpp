#include "factory/deckFactory.h"

std::unique_ptr<Deck> DeckFactory::create(const std::string &pathFolder) {
  std::vector<std::unique_ptr<Card>> cards;
  for (const auto &path : listFiles(pathFolder)) {
    nlohmann::json cardJson = load(path);
   
    int quantity = cardJson.value("quantity", 1);
    for (int i = 0; i < quantity; ++i) {
      cards.push_back(CardFactory::create(cardJson));
    }
  }

  auto deck = std::make_unique<Deck>();
  deck->setPileDraw(std::move(cards));
  deck->shuffle();
  return deck;
}
