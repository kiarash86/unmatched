#pragma once
#include <memory>
#include <nlohmann/json.hpp>
#include "engine/conditions/condition.h"
#include "engine/conditions/proximityCondition.h"
#include "engine/conditions/combatOutcomeCondition.h"
#include "engine/conditions/isTeamAdjacentCondition.h"
#include "engine/conditions/isSherlockWatsonCardCondition.h"
#include "utility/file.h"

class ConditionFactory {
public:

  static std::unique_ptr<Condition> create(const std::string &condition, int distance = 1);
};
