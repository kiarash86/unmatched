#pragma once
#include "engine/gameData.h"
#include "engine/conditions/condition.h"
#include <memory>
#include <vector>

class Query {
protected:
  std::vector<std::unique_ptr<Condition>> conditions;

public:
  Query() = default;
  virtual ~Query() = default;

  void addCondition(std::unique_ptr<Condition> cond) {
    conditions.push_back(std::move(cond));
  }
  virtual int get(gameData &gameData) = 0;
};
