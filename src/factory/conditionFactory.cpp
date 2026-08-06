#include "factory/conditionFactory.h"
#include "utility/exceptions.h"

std::unique_ptr<Condition> ConditionFactory::create(const std::string &condition, int distance) {
  if (condition == "isNearEnemyCondition" || condition == "near_enemy" ||
      condition == "same_scope_with_enemy") {
    return std::make_unique<ProximityCondition>(&gameData::enemy, distance);
  }
  if (condition == "near_each_other") {
    return std::make_unique<IsTeamAdjacentCondition>(distance);
  }
  if (condition == "near_hero") {
    return std::make_unique<ProximityCondition>(&gameData::self, distance);
  }
  if (condition == "near_target") {
    return std::make_unique<ProximityCondition>(&gameData::target, distance);
  }
  if (condition == "isLossed") {
    return std::make_unique<CombatOutcomeCondition>(CombatOutcome::Lost);
  }
  if (condition == "won_the_war") {
    return std::make_unique<CombatOutcomeCondition>(CombatOutcome::Won);
  }
  if (condition == "isWinnerSelf") {
    return std::make_unique<CombatOutcomeCondition>(CombatOutcome::Won);
  }
  if (condition == "is_sherlock_watson_card") {
    return std::make_unique<IsSherlockWatsonCardCondition>();
  }
  if (condition == "self_on_fog_token") {
    return std::make_unique<IsOnFogTileCondition>(false);
  }
  if (condition == "self_started_turn_on_fog_token") {
    return std::make_unique<IsOnFogTileCondition>(true);
  }
  return nullptr;
}
