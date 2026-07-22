#include "model/card.h"

Card::Card() = default;

int Card::getBoost() const { return boost; } //boost 

std::string Card::getName() const { return name; } //name

TypeOfCard Card::getCardType() const { return type; } //type of card

TypeOfEvent Card::getEventType() const { return eventType; }//event type

std::vector<std::unique_ptr<Effect>> &Card::getEffects() { return effects; }// effects

std::string Card::getImgSource() const { return imgSource; } //img path

TypeOfPerformer Card::getPerformer() const { return performer; } // performer

std::string Card::getPerformerName() const { return performerName; }// name of performer

int Card::getAttackStat() const { return attackStat; } // value

int Card::getDefStat() const { return defStat; } // value

int Card::getPredictedValue() const { return predictedValue; } // just for extra

void Card::setPredictedValue(const int &newPredictedValue) { // set just for extra
  predictedValue = newPredictedValue;
}

bool Card::needsTarget() const { // card need someone?
  for (auto &effect : effects) {
    if (effect->needsTarget())
      return true;
  }
  return false;
}

bool Card::needsPrediction() const { // card need to predicted someone?
  for (auto &effect : effects) {
    if (effect->requiresPrediction())
      return true;
  }
  return false;
}

void Card::setBoost(const int &newBoost) { boost = newBoost; } //set boost

void Card::setName(const std::string &newName) { name = newName; } // set name

void Card::setCardType(const TypeOfCard &newType) { type = newType; } //set type off card

void Card::setEventType(const TypeOfEvent &newEventType) {  // set event type
  eventType = newEventType;
}

void Card::setImgSource(const std::string &newImgSource) { // set path of img
  imgSource = newImgSource;
}

void Card::setPerformer(const TypeOfPerformer &newPerformer) { // set performer
  performer = newPerformer;
}

void Card::setPerformerName(const std::string &newPerformerName) { // set name of performer
  performerName = newPerformerName;
}

void Card::setAttackStat(const int &newAttackStat) { // set value
  attackStat = newAttackStat;
}

void Card::setDefStat(const int &newDefStat) { defStat = newDefStat; } // set value

void Card::modifyDefStat(const int &amount) { defStat += amount; } // modify value

int Card::getValue() const { return value; } // get value

void Card::setValue(const int &newValue) { value = newValue; } // set value

void Card::modifyValue(const int &amount) { value += amount; } // modify value

void Card::addEffect(std::unique_ptr<Effect> eff) { // adding effect
  effects.push_back(std::move(eff));
}

void Card::cancelEffects(int howMany) { // cancell effect if someone used 'feint'
  if (howMany < 0) {
    cancelledEffects = -1; // all
    return;
  }
  if (howMany == 0)
    return;

  cancelledEffects = (cancelledEffects < 0) ? -1 : cancelledEffects + howMany;
}

bool Card::consumeCancellation() {  // all effects? 1 effect or...(negative is all)

  if (cancelledEffects == 0)
    return false;
  if (cancelledEffects > 0)
    cancelledEffects--;

  return true;
}

void Card::resetCancellation() { cancelledEffects = 0; } // start from first

