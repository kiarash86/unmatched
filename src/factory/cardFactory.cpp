#include "../../include/factory/cardFactory.h"

std::unique_ptr<Card> CardFactory::create(const nlohmann::json &card) {

  std::unique_ptr<Card> crd;

  crd->setName(card["name"]);
  crd->setImgSource(card["img"]);
  crd->setBoost(card["boost"]);
  crd->setPerformer(card["performer"]);
  crd->setCardType(card["type"]);
  crd->setEventType(card["eventType"]);
  for (const auto &eff : card["effects"]) {

    crd->addEffect(EffectFactory::create(eff));
  }

  return crd;
}
