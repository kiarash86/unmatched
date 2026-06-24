#include <memory>
#include "engine/conditions/condition.h"
#include "engine/conditions/isNearEnemyCondition.h"
#include <nlohmann/json.hpp>
#include "utility/file.h"
class ConditionFactory
{
private:

public:

static std::unique_ptr<Condition> create(const std::string & condition);


    
};