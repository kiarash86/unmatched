#include "model/card.h"

Card::Card() = default;

int Card::getBoost() const { return boost; }
std::string Card::getName() const { return name; }
TypeOfCard Card::getCardType() const { return type; }
TypeOfEvent Card::getEventType() const { return eventType; }
std::vector<std::unique_ptr<Effect>> &Card::getEffects() { return effects; }
std::string Card::getImgSource() const { return imgSource; }
TypeOfPerformer Card::getPerformer() const { return performer; }
std::string Card::getPerformerName() const { return performerName; }
int Card::getAttackStat() const { return attackStat; }
int Card::getDefStat() const { return defStat; }
bool Card::needsTarget() const {
  for (auto &effect : effects) {
    if (effect->needsTarget()) return true;
  }
  return false;
}
void Card::setBoost(const int &newBoost) { boost = newBoost; }
void Card::setName(const std::string &newName) { name = newName; }
void Card::setCardType(const TypeOfCard &newType) { type = newType; }
void Card::setEventType(const TypeOfEvent &newEventType) {
  eventType = newEventType;
}
void Card::setImgSource(const std::string &newImgSource) {
  imgSource = newImgSource;
}
void Card::setPerformer(const TypeOfPerformer &newPerformer) {
  performer = newPerformer;
}
void Card::setPerformerName(const std::string &newPerformerName) {
  performerName = newPerformerName;
}
void Card::setAttackStat(const int &newAttackStat) { attackStat = newAttackStat; }
void Card::setDefStat(const int &newDefStat) { defStat = newDefStat; }
void Card::modifyDefStat(const int &amount) { defStat += amount; }

int Card::getValue() const { return value; }
void Card::setValue(const int &newValue) { value = newValue; }
void Card::modifyValue(const int &amount) { value += amount; }

void Card::addEffect(std::unique_ptr<Effect> eff) {
  effects.push_back(std::move(eff));
}
