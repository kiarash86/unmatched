#pragma once
#include "engine/gameData.h"

class Fighter;

class Condition {
public:
  Condition() = default;
  virtual ~Condition() = default;
  virtual bool check(gameData &gameData, Fighter *fighter = nullptr) = 0;
};
