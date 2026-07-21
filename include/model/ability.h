#pragma once
#include "typeOfEvent.h"
#include <memory>
#include <string>
#include <vector>

class Effect;
struct gameData;

class Ability {
private:
  std::string name;
  std::string description;
  TypeOfEvent trigger{TypeOfEvent::none};
  std::vector<std::unique_ptr<Effect>> effects;

public:
  Ability();
  ~Ability();

  void setName(const std::string &);
  std::string getName() const;
  void setDescription(const std::string &);
  std::string getDescription() const;
  void setTrigger(const TypeOfEvent &);
  TypeOfEvent getTrigger() const;

  void addEffect(std::unique_ptr<Effect> effect);
  const std::vector<std::unique_ptr<Effect>> &getEffects() const;
  
  void activate(gameData &data, const TypeOfEvent &event);
};
