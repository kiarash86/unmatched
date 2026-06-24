#include "../../include/factory/conditionFactory.h"

std::unique_ptr<Condition> ConditionFactory::create(const std::string &condition) {

    if (condition == "isNearEnemyCondition")
    {

      auto cnd = std::make_unique<IsNearEnemyCondition>();
       return cnd;
    }
}
