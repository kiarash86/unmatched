#pragma once
#include <memory>
#include <nlohmann/json.hpp>
#include "utility/file.h"
#include "engine/queries/query.h"
#include "engine/queries/countfighter.h"
#include "engine/queries/cardBoost.h"
#include "factory/conditionFactory.h"

class QueryFactory {
public:
  static std::unique_ptr<Query> create(const nlohmann::json &query);
};
