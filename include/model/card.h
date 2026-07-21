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
  // Raw "performer" string from the json (e.g. "sherlock", "watson",
  // "dracula"), kept alongside the coarse TypeOfPerformer enum.
  // TypeOfPerformer only captures a card's *role* requirement
  // (fighter/hero/sidekick) and can't tell "sherlock" from "watson"
  // from "fighter" -- conditions that need a specific character's
  // identity (e.g. is_sherlock_watson_card) need this instead.
  std::string performerName;
  int boost{};
  std::string imgSource;
  TypeOfEvent eventType{};
  // Raw attack/defense stats from the card's json (top-level "attack"
  // and "def" keys, or "attack/def" for multipurpose cards, which sets
  // both). Read by GameManager::resolveCombat() to seed `value` (the
  // card's working combat number) once it's committed to a round --
  // see Card::setValue()/modifyValue() and DefEffect for how that
  // number can then move before the round is compared.
  int attackStat{};
  int defStat{};
  std::vector<std::unique_ptr<Effect>> effects;

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
  // True if any of this card's effects reads gameData.target (e.g. a
  // damage or position-exchange effect) -- meaning it needs a specific
  // fighter chosen rather than the simplified "next hero in turn
  // order" default. See GameManager::playCard(Card*) vs. the full
  // self/target/enemy overload.
  bool needsTarget() const;
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
};
