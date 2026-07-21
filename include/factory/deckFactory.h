#pragma once
#include <memory>
#include "model/card.h"
#include "model/deck.h"
#include <nlohmann/json.hpp>
#include "utility/file.h"
#include "cardFactory.h"

class DeckFactory {
public:
  static std::unique_ptr<Deck> create(const std::string &pathFolder);
};
