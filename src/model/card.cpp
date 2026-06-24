#include "../../include/model/card.h"

int Card::getBoost() const { return boost; }
std::string Card::getName() const { return name; }
TypeOfCard Card::getCardType() const { return type; }
TypeOfEvent Card::getEventType() const { return eventType; }
std::vector<std::unique_ptr<Effect>> &Card::getEffects() { return effects; }
std::string Card::getImgSource() const { return imgSource; }
TypeOfPerformer Card::getPerformer() const { return performer; }
void Card::setBoost(const int &newBoost) { boost = newBoost; }
void Card::setName(std::string &newName) { name = newName; }
void Card::setCardType(const TypeOfCard &newType) { type = newType; }
void Card::setEventType(const TypeOfEvent &newEventType) {
  eventType = newEventType;
}
void Card::setEffects(const std::vector<Effect> &newEffects) {
  // TODO
  // what should i do here?
}
void Card::setImgSource(const std::string &newImgSource) {
  imgSource = newImgSource;
}
void Card::setPerformer(const TypeOfPerformer &newPerformer) {
  performer = newPerformer;
}

int Card::getValue() const { return value; }
void Card::setValue(const int &newValue) { value = newValue; }
void Card::modifyValue(const int &amount) { value += amount; }


void Card::addEffect(std::unique_ptr<Effect> eff) {
    effects.push_back(eff);
  }