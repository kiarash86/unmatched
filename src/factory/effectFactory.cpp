#include "factory/effectFactory.h"
#include "factory/queryFactory.h"

namespace {

int baseAmount(const nlohmann::json &effect, const char *k) {
  if (!effect.contains(k) || !effect[k].is_number()) {
    return 0;
  }
  return effect[k].get<int>();
}
}

std::unique_ptr<Effect> EffectFactory::create(const nlohmann::json &effect) {
  std::unique_ptr<Effect> eff;

  std::string type = effect.value("type", effect.value("name", ""));

  if (type == "modify") {
    eff = std::make_unique<ModifierEffect>(baseAmount(effect, "howMuch"));
  } else if (type == "attack" || type == "dmg" || type == "damage") {
    eff = std::make_unique<DmgEffect>(baseAmount(effect, "howMuch"));
  } else if (type == "draw_card" || type == "draw") {
    int amount = baseAmount(effect, "howMany");
    eff = std::make_unique<DrawEffect>(amount > 0 ? amount : 1);
  } else if (type == "move") {
    eff = std::make_unique<MoveEffect>();
  } else if (type == "remove_card") {
    int amount = baseAmount(effect, "howMany");
    eff = std::make_unique<RemoveCardEffect>(amount > 0 ? amount : 1);
  } else if (type == "remove_effect") {
    eff = std::make_unique<RemoveEffectEffect>();
  } else if (type == "see_hand" || type == "seeHand") {

    eff = std::make_unique<SeeHandEffect>();
  } else if (type == "position_exchange") {
    eff = std::make_unique<PositionExchangeEffect>();
  } else if (type == "add") {
    std::string toWhat = effect.value("toWhat", "");
    std::string toWho = effect.value("towho", effect.value("who", ""));
    eff = std::make_unique<AddEffect>(toWhat, toWho, baseAmount(effect, "howMuch"));
  } else if (type == "def") {
    eff = std::make_unique<DefEffect>(baseAmount(effect, "howMuch"));
  } else if (type == "change_value") {
    bool targetEnemy = effect.value("onWho", "") == "enemy";
    bool useBoost = effect.value("changeWith", "") == "boost";
    eff = std::make_unique<ChangeValueEffect>(targetEnemy, useBoost, baseAmount(effect, "changeWith"));
  } else if (type == "choose_place") {
    eff = std::make_unique<ChoosePlaceEffect>(effect.value("who", "self"));
  }

  if (!eff) {
    return nullptr; 
  }

  if (effect.contains("condition") && effect["condition"].is_string()) {
    auto cond = ConditionFactory::create(effect["condition"].get<std::string>(),
                                          effect.value("distance", 1));
    if (cond) {
      eff->addCondition(std::move(cond));
    }
  }
  if (effect.contains("conditions")) {
    for (auto &&cnd : effect["conditions"]) {
      auto cond = ConditionFactory::create(cnd.get<std::string>(), effect.value("distance", 1));
      if (cond) {
        eff->addCondition(std::move(cond));
      }
    }
  }

  if (effect.contains("queries")) {
    for (auto &&qry : effect["queries"]) {
      auto query = QueryFactory::create(qry);
      if (query) {
        eff->addQuery(std::move(query));
      }
    }
  }

  return eff;
}
// at first with else if making a effect then addng conditions and then queries
// of course with factories they have