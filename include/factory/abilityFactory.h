#pragma once
#include <memory>
#include <string>
#include "model/ability.h"

class AbilityFactory {
public:

  static std::unique_ptr<Ability> create(const std::string &pathFolder);
};
