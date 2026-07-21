#include "model/ability.h"
#include "engine/effects/effect.h"

Ability::Ability() = default;
Ability::~Ability() = default;

void Ability::setName(const std::string &newName) { name = newName; }
std::string Ability::getName() const { return name; }
void Ability::setDescription(const std::string &newDesc) { description = newDesc; }
std::string Ability::getDescription() const { return description; }
void Ability::setTrigger(const TypeOfEvent &event) { trigger = event; }
TypeOfEvent Ability::getTrigger() const { return trigger; }

void Ability::addEffect(std::unique_ptr<Effect> effect) {
  effects.push_back(std::move(effect));
}

const std::vector<std::unique_ptr<Effect>> &Ability::getEffects() const {
  return effects;
}

void Ability::activate(gameData &data, const TypeOfEvent &event) {
  if (trigger != event) {
    return;
  }
  for (auto &effect : effects) {
    effect->execute(data);
  }
}
