#pragma once
#include <memory>
#include "../model/hero.h"
#include <nlohmann/json.hpp>
#include "../utility/file.h"
#include "sidekickFactory.h"
#include "deckFactory.h"

class HeroFactory {
public:
  // name is the hero's data folder name
  static std::unique_ptr<Hero> create(const std::string &name);
};
