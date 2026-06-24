#pragma once
#include "typeOfCard.h"
#include "typeOfEvent.h"
#include "typeOfPerformer.h"
#include <memory>
#include <string>
#include <vector>
class Effect;
class Card {
private:
  int value;
  std::string name;
  TypeOfCard type;
  TypeOfPerformer performer;
  int boost;
  std::string imgSource;
  TypeOfEvent eventType;
  std::vector<std::unique_ptr<Effect>> effects;

public:
  int getValue() const;
  void setValue(const int&);
  void modifyValue(const int &);
  int getBoost() const;
  std::string getName() const;
  TypeOfCard getCardType() const;
  TypeOfEvent getEventType() const;
  std::vector<std::unique_ptr<Effect>> &getEffects();
  std::string getImgSource() const;
  TypeOfPerformer getPerformer() const;
  void setBoost(const int &);
  void setName(std::string &);
  void setCardType(const TypeOfCard &);
  void setEventType(const TypeOfEvent &);
  void setEffects(const std::vector<Effect> &);
  void setImgSource(const std::string &);
  void setPerformer(const TypeOfPerformer &);
  Card();
  ~Card() = default;
};
