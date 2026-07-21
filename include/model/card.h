#pragma once
#include "typeOfCard.h"
#include "typeOfEvent.h"
#include "typeOfPerformer.h"
#include "engine/effects/effect.h"
#include <memory>
#include <string>
#include <vector>

class Card {
private:
  int value{};
  std::string name;
  TypeOfCard type{};
  TypeOfPerformer performer{};

  std::string performerName;
  int boost{};
  std::string imgSource;
  TypeOfEvent eventType{};

  int attackStat{};
  int defStat{};
  std::vector<std::unique_ptr<Effect>> effects;

  int cancelledEffects{0};

  bool targetsAnyFighter{false};

public:
  int getValue() const;
  void setValue(const int &);
  void modifyValue(const int &);
  int getBoost() const;
  std::string getName() const;
  TypeOfCard getCardType() const;
  TypeOfEvent getEventType() const;
  std::vector<std::unique_ptr<Effect>> &getEffects();
  std::string getImgSource() const;
  TypeOfPerformer getPerformer() const;
  std::string getPerformerName() const;
  int getAttackStat() const;
  int getDefStat() const;

  bool needsTarget() const;
  bool isTargetsAnyFighter() const { return targetsAnyFighter; }
  void setTargetsAnyFighter(bool v) { targetsAnyFighter = v; }
  void setBoost(const int &);
  void setName(const std::string &);
  void setCardType(const TypeOfCard &);
  void setEventType(const TypeOfEvent &);
  void setImgSource(const std::string &);
  void setPerformer(const TypeOfPerformer &);
  void setPerformerName(const std::string &);
  void setAttackStat(const int &);
  void setDefStat(const int &);
  void modifyDefStat(const int &);
  Card();
  ~Card() = default;

  void addEffect(std::unique_ptr<Effect> eff);

  void cancelEffects(int howMany);

  bool consumeCancellation();

  void resetCancellation();
};
