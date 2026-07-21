#pragma once
#include <memory>
#include <nlohmann/json.hpp>
#include "engine/conditions/condition.h"
#include "engine/conditions/isNearEnemyCondition.h"
#include "engine/conditions/isTeamAdjacentCondition.h"
#include "engine/conditions/isLossedCondition.h"
#include "engine/conditions/isSherlockWatsonCardCondition.h"
#include "engine/conditions/wonTheWarCondition.h"
#include "engine/conditions/isWinnerSelfCondition.h"
#include "engine/conditions/nearHeroCondition.h"
#include "engine/conditions/isNearTargetCondition.h"
#include "utility/file.h"

class ConditionFactory {
public:
 
  static std::unique_ptr<Condition> create(const std::string &condition, int distance = 1);
};
