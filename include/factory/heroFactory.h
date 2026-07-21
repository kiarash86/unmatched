#pragma once
#include <memory>
#include "../model/hero.h"
#include <nlohmann/json.hpp>
#include "../utility/file.h"
#include "sidekickFactory.h"
#include "deckFactory.h"
#include "abilityFactory.h"

class HeroFactory {
public:
  // `name` is the hero's data folder name, e.g. "Dracula" or
  // "SherlockHolms" (matches the folders under data/).
  static std::unique_ptr<Hero> create(const std::string &name);
};
