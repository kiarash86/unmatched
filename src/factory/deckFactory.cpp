#include "factory/deckFactory.h"
#include "utility/exceptions.h"

std::unique_ptr<Deck> DeckFactory::create(const std::string &pathFolder) {
  try {
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
  } catch (const AppException &e) {
    throw FactoryException("Failed to build Deck from '" + pathFolder + "': " + e.what());
  } catch (const nlohmann::json::exception &e) {
    throw FactoryException("Failed to build Deck from '" + pathFolder +
                            "': malformed data (" + e.what() + ")");
  }
}
