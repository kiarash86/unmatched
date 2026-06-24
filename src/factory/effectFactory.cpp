#include "../../include/factory/effectFactory.h"

std::unique_ptr<Effect> EffectFactory::create(const nlohmann::json &effect) {
  auto eff = std::make_unique<Effect>();

  
  if (effect["name"] == "modify")
  {
    eff = std::move(std::make_unique<ModifierEffect>(int(effect["value"])));
  }
  
  for (auto &&cnd : effect["conditions"]) {
    
    eff->addCondition(std::move(ConditionFactory::create(std::string(cnd))));
  }

  for (auto &&qry : effect["queries"]) {
    eff->addQuery(QueryFactory::create(qry));
  }


  return eff;
}
