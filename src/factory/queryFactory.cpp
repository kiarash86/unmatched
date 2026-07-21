#include "factory/queryFactory.h"
#include "libraries/magic_enum.hpp"
#include <cctype>



std::unique_ptr<Query> QueryFactory::create(const nlohmann::json &query) {
  std::unique_ptr<Query> qry;

  std::string name = query.value("name", "");
  if (name == "countFighter") {
    qry = std::make_unique<CountFighter>(query.value("type", "Fighter"));
  } else if (name == "cardBoost") {
    bool useEnemy = query.value("who", "self") == "enemy";
    qry = std::make_unique<CardBoost>(useEnemy);
  } else if (name == "countRemovedCards") {
    bool useEnemy = query.value("who", "self") == "enemy";
    qry = std::make_unique<CountRemovedCards>(useEnemy);
  }

  if (!qry) {
    return nullptr; 

  if (query.contains("conditions")) {
    for (auto &&cnd : query["conditions"]) {
      auto cond = ConditionFactory::create(cnd.get<std::string>());
      if (cond) {
        qry->addCondition(std::move(cond));
      }
    }
  }

  return qry;
}
