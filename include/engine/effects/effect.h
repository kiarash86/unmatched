#pragma once
#include <memory>
#include <vector>
#include "engine/gameData.h"
#include "engine/conditions/condition.h"
#include "engine/queries/query.h"

class Effect {
protected:
  std::vector<std::unique_ptr<Condition>> conditions;
  std::vector<std::unique_ptr<Query>> queries;


  bool conditionsMet(gameData &gameData) {
    for (auto &&cond : conditions) {
      if (!cond->check(gameData)) {
        return false;
      }
    }
    return true;
  }

  int sumQueries(gameData &gameData) {
    int total = 0;
    for (auto &&query : queries) {
      total += query->get(gameData);
    }
    return total;
  }

public:
  Effect() = default;
  virtual ~Effect() = default;

  void addCondition(std::unique_ptr<Condition> cond) {
    conditions.push_back(std::move(cond));
  }
  void addQuery(std::unique_ptr<Query> query) {
    queries.push_back(std::move(query));
  }

  virtual bool needsTarget() const { return false; }

  virtual void execute(gameData &gameData) = 0;
};
