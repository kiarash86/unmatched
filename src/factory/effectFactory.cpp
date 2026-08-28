#include "factory/effectFactory.h"
#include "factory/queryFactory.h"
#include "utility/exceptions.h"

namespace {

int baseAmount(const nlohmann::json &effect, const char *k) {
  if (!effect.contains(k) || !effect[k].is_number()) {
    return 0;
  }
  return effect[k].get<int>();
}
}

std::unique_ptr<Effect> EffectFactory::create(const nlohmann::json &effect) {
  std::string type = effect.value("type", effect.value("name", ""));

  try {
  std::unique_ptr<Effect> eff;
  bool conditionHandledInline = false; 
                                    

  if (type == "modify" || type == "def") {

    eff = std::make_unique<ChangeValueEffect>(false, false, baseAmount(effect, "howMuch"));
  } else if (type == "attack" || type == "dmg" || type == "damage") { //  part of attacking and dmg and damage
    auto dmgEff = std::make_unique<DmgEffect>(baseAmount(effect, "howMuch"),
                                               effect.value("toWhat", ""));
   if (effect.contains("howMuchIfCondition") && effect["howMuchIfCondition"].is_number() &&
        effect.contains("condition") && effect["condition"].is_string()) {
      auto boostCond = ConditionFactory::create(effect["condition"].get<std::string>(),
                                                 effect.value("distance", 1));
      if (boostCond) {
        dmgEff->setConditionalValue(std::move(boostCond),
                                     effect["howMuchIfCondition"].get<int>());
        conditionHandledInline = true;
      }
    }
    eff = std::move(dmgEff);
  } else if (type == "draw_card" || type == "draw") {
    int amount = baseAmount(effect, "howMany");
    std::string toWho = effect.value("toWho", effect.value("who", "self"));
    auto drawEff = std::make_unique<DrawEffect>(amount > 0 ? amount : 1, toWho);

    if (effect.contains("else") && effect["else"].is_object()) {
      const auto &elseObj = effect["else"];
      int elseAmount = baseAmount(elseObj, "howMany");
      std::string elseToWho = elseObj.value("toWho", elseObj.value("who", toWho));
      drawEff->setElse(elseAmount > 0 ? elseAmount : 1, elseToWho);
    }

    eff = std::move(drawEff);
  } else if (type == "move") {
    int howMany = (effect.contains("howMany") && effect["howMany"].is_number())
                      ? effect["howMany"].get<int>()
                      : -1; 
    std::string whichOne = effect.value("whichOne", "self");
    int distance = (effect.contains("distance") && effect["distance"].is_number())
                       ? effect["distance"].get<int>()
                       : -1;
    std::string toWhere = effect.value("toWhere", "none");
    bool allowStay = effect.value("allowStay", false);
    std::string performer = effect.value("performer", "self");
    eff = std::make_unique<MoveEffect>(howMany, whichOne, distance, toWhere, allowStay, performer);
  } else if (type == "remove_card") {
    int amount = baseAmount(effect, "howMany");
    if (amount == 0) {
      amount = baseAmount(effect, "amount");
    }
    if (amount == 0) {
      amount = 1;
    }

    bool fromEnemy = effect.value("from", "self") == "enemy";
    bool userChosen = effect.value("how", "random") == "user_chosen";

    int bonusPerCard = 0;
    if (effect.contains("bonusPerCard") && effect["bonusPerCard"].is_object()) {
      bonusPerCard = effect["bonusPerCard"].value("amount", 0);
    } // for beastform, we use bonuspercard here 

    eff = std::make_unique<RemoveCardEffect>(amount, fromEnemy, userChosen, bonusPerCard);
  } else if (type == "remove_effect") {
    int howMany = effect.contains("howMany") && effect["howMany"].is_number()
                      ? effect["howMany"].get<int>()
                      : -1;
    std::string whichOne = effect.value("whichOne", "all");
    bool targetEnemy = effect.value("who", "enemy") == "enemy";
    bool targetsAbility = effect.value("target", "card") == "ability"; //using this to cancel effect with abiltiy(\)
    eff = std::make_unique<RemoveEffectEffect>(howMany, whichOne, targetEnemy, targetsAbility);
  } else if (type == "see_hand" || type == "seeHand") {

    eff = std::make_unique<SeeHandEffect>();
  } else if (type == "position_exchange") {
    eff = std::make_unique<PositionExchangeEffect>();
  } else if (type == "add") {
    std::string toWhat = effect.value("toWhat", "");
    std::string toWho = effect.value("towho", effect.value("who", ""));
    eff = std::make_unique<AddEffect>(toWhat, toWho, baseAmount(effect, "howMuch"));
  } else if (type == "change_value") {
    bool targetEnemy = effect.value("onWho", "") == "enemy";
    bool useBoost = effect.value("changeWith", "") == "boost";
    eff = std::make_unique<ChangeValueEffect>(targetEnemy, useBoost, baseAmount(effect, "changeWith"));
  } else if (type == "set_value" || type == "set_value_fixed") {
    bool targetEnemy = effect.value("who", "") == "enemy";
    bool unmodifiable = effect.value("unmodifiable", false);
    eff = std::make_unique<SetValueEffect>(targetEnemy, baseAmount(effect, "howMuch"), unmodifiable);
  } else if (type == "remove_from_board") {
    eff = std::make_unique<RemoveFromBoardEffect>(effect.value("whichOne", "self"));
  } else if (type == "place_on_deck") {
    int amount = baseAmount(effect, "howMany");
    if (amount == 0) {
      amount = 1;
    }
    bool fromEnemy = effect.value("from", "self") == "enemy";
    eff = std::make_unique<PlaceOnDeckEffect>(amount, fromEnemy);
  } else if (type == "choose_effect") {
    std::vector<std::unique_ptr<Effect>> options;
    std::vector<std::string> labels;
    if (effect.contains("options")) {
      for (auto &&opt : effect["options"]) {
        if (auto built = EffectFactory::create(opt)) {
     labels.push_back(opt.value(
              "label", opt.value("type", opt.value("name", "Option"))));
          options.push_back(std::move(built));
        }
      }
    }
    eff = std::make_unique<ChooseEffectEffect>(std::move(options), std::move(labels));
  } else if (type == "opponent_choice") {
    std::unique_ptr<Effect> choiceEffect;
    std::unique_ptr<Effect> declineEffect;
    if (effect.contains("choice")) {
      choiceEffect = EffectFactory::create(effect["choice"]);
    }
    if (effect.contains("onDecline")) {
      declineEffect = EffectFactory::create(effect["onDecline"]);
    }
    eff = std::make_unique<OpponentChoiceEffect>(std::move(choiceEffect), std::move(declineEffect));
  } else if (type == "choose_place") {
    eff = std::make_unique<ChoosePlaceEffect>(effect.value("who", "self"),
                                               effect.value("when", "immediate"));
  } else if (type == "revive") {
    eff = std::make_unique<ReviveEffect>(effect.value("whichOne", effect.value("who", "sidekick")));
  } else if (type == "predict") {
    eff = std::make_unique<PredictEffect>();
  }

  if (!eff) {
    return nullptr; 
  }

  if (!conditionHandledInline && effect.contains("condition") &&
      effect["condition"].is_string()) { // adding conditions here
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

  if (effect.contains("queries")) { //adding queries here
    for (auto &&qry : effect["queries"]) {
      auto query = QueryFactory::create(qry);
      if (query) {
        eff->addQuery(std::move(query));
      }
    }
  }

  return eff;
  } catch (const AppException &e) {
    throw FactoryException("Failed to build Effect of type '" + type + "': " + e.what());
  } catch (const nlohmann::json::exception &e) {
    throw FactoryException("Failed to build Effect of type '" + type +
                            "': malformed data (" + e.what() + ")");
  }
}
// at first with else if making a effect then add conditions and then queries
// of course with factories they have