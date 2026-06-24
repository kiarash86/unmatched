#include "../../include/factory/queryFactory.h"

std::unique_ptr<Query> QueryFactory::create(const nlohmann::json &query) {
  auto qry = std::make_unique<Query>();

  
  if (query["name"] == "countFighter")
  {
    qry = std::move(std::make_unique<CountFighter>(query["type"]));
  }
  
  for (auto &&cnd : query["conditions"]) {
    
    qry->addCondition(std::move(ConditionFactory::create(std::string(cnd))));
  }


  return qry;
}
