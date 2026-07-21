#include "factory/cardFactory.h"
#include "libraries/magic_enum.hpp"
#include <cctype>

namespace {
template <typename Enum>
Enum enumConvert(const std::string &raw, Enum fallback) {
  std::string c = raw;
  if (!c.empty()) {
    c[0] = (char)std::tolower((unsigned char)c[0]);
  }
  auto parsed = magic_enum::enum_cast<Enum>(c);
  return parsed.value_or(fallback);
}
}

std::unique_ptr<Card> CardFactory::create(const nlohmann::json &card) {
  auto crd = std::make_unique<Card>();

  crd->setName(card.value("name", ""));
  crd->setImgSource(card.value("img", ""));
  crd->setBoost(card.value("boost", 0));
  crd->setPerformer(enumConvert(card.value("performer", "fighter"), TypeOfPerformer::fighter));
 
  crd->setPerformerName(card.value("performer", ""));
  crd->setCardType(enumConvert(card.value("type", "multipurpose"), TypeOfCard::multipurpose));
  crd->setEventType(enumConvert(card.value("eventType", "none"), TypeOfEvent::none));
  if (card.contains("attack") && card["attack"].is_number()) {
    crd->setAttackStat(card["attack"].get<int>());
  }
  if (card.contains("def") && card["def"].is_number()) {
    crd->setDefStat(card["def"].get<int>());
  }
  if (card.contains("attack/def") && card["attack/def"].is_number()) {
    int both = card["attack/def"].get<int>();
    crd->setAttackStat(both);
    crd->setDefStat(both);
  }

  if (card.contains("effects")) {
    for (const auto &eff : card["effects"]) {
      auto built = EffectFactory::create(eff);
      if (built) {
        crd->addEffect(std::move(built));
      }
    }
  }

  return crd;
}
