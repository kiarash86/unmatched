#pragma once
#include "fighter.h"

class Sidekick : public Fighter {
public:
  Sidekick();
  ~Sidekick() override = default;

  TypeOfFighter getFighterType() const override { return TypeOfFighter::Sidekick; }
};
