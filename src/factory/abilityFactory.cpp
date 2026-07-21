#include "factory/abilityFactory.h"
#include "factory/effectFactory.h"
#include "utility/file.h"
#include "libraries/magic_enum.hpp"
#include <filesystem>

std::unique_ptr<Ability> AbilityFactory::create(const std::string &pathFolder) {
  auto ability = std::make_unique<Ability>();

  auto files = listFiles(pathFolder);
  if (files.empty()) {
    return ability;
  } // no ability

  nlohmann::json data = load(files.front());

  std::string abilityName = std::filesystem::path(files.front()).stem().string();
  ability->setName(abilityName);

  if (data.contains("when")) { // same as effects
    auto trigger = magic_enum::enum_cast<TypeOfEvent>(data["when"].get<std::string>());
    ability->setTrigger(trigger.value_or(TypeOfEvent::none));
  }

  if (auto built = EffectFactory::create(data)) {
    ability->addEffect(std::move(built));
  }

  return ability;
}
