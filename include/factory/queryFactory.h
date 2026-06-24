#include <memory>
#include <nlohmann/json.hpp>
#include "../utility/file.h"
#include "engine/queries/query.h"
#include"engine/queries/countfighter.h"
#include "factory/conditionFactory.h"
class QueryFactory
{
private:

public:

static std::unique_ptr<Query> create(const nlohmann::json & query);

};