#pragma once
#include <functional>
#include <memory>
#include <vector>
#include "engine/gameData.h"
#include "engine/conditions/condition.h"
#include "engine/queries/query.h"

class Effect {
protected:
  std::vector<std::unique_ptr<Condition>> conditions;
  std::vector<std::unique_ptr<Query>> queries;

  bool conditionsMet(gameData &gameData) { // checking all conds
    for (auto &&cond : conditions) {
      if (!cond->check(gameData)) {
        return false;
      }
    }
    return true;
  }

  int sumQueries(gameData &gameData) { // adding all queries
    int total = 0;
    for (auto &&query : queries) {
      total += query->get(gameData);
    }
    return total;
  }

  virtual void executeImmediate(gameData &) {}

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
  virtual bool requiresAdjacentTarget() const { return false; }

  virtual bool requiresPrediction() const { return false; } // for that extra card(i dont remember name)

  virtual void execute(gameData &gameData, std::function<void()> onDone) { //there is excute for defalut and we develop this immediate excture to customize excute
    executeImmediate(gameData);
    if (onDone) onDone();
  }

  // for effets that has effect in future
  // for example they excuted at the first of next round and...
  virtual void executeDeferred(gameData &gameData, std::function<void()> onDone) {
    execute(gameData, std::move(onDone));
  }
};
