#include "factory/conditionFactory.h"
#include "utility/exceptions.h"

std::unique_ptr<Condition> ConditionFactory::create(const std::string &condition, int distance) {
  if (condition == "isNearEnemyCondition" || condition == "near_enemy" ||
      condition == "same_scope_with_enemy") {
    return std::make_unique<IsNearEnemyCondition>(distance);
  }
  if (condition == "near_each_other") {

    return std::make_unique<IsTeamAdjacentCondition>(distance);
  }
  if (condition == "near_hero") {
    return std::make_unique<NearHeroCondition>(distance);
  }
  if (condition == "near_target") {
    return std::make_unique<IsNearTargetCondition>(distance);
  }
  if (condition == "isLossed") {
    return std::make_unique<IsLossedCondition>();
  }
  if (condition == "won_the_war") {
    return std::make_unique<WonTheWarCondition>(distance);
  }
  if (condition == "isWinnerSelf") {
    return std::make_unique<IsWinnerSelfCondition>();
  }
  if (condition == "is_sherlock_watson_card") {
    return std::make_unique<IsSherlockWatsonCardCondition>();
  }
  return nullptr;
}
