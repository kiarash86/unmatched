#include "factory/sidekickFactory.h"

namespace {
void setStats(const nlohmann::json &stats, Sidekick *sidekick) {
  sidekick->setName(stats.value("name", ""));
  sidekick->setImgSource(stats.value("img", ""));
  sidekick->setHealth(stats.value("health", 1));
  sidekick->setMaxHealth(stats.value("health", 1));

  sidekick->setMovement(stats.value("movement", stats.value("moves", 0)));
  std::string attackType = stats.value("attackType", "melee");

  if (attackType == "ranged") {
    sidekick->setTypeOfAttack(TypeOfAttack::ranged);
  } else {
    sidekick->setTypeOfAttack(TypeOfAttack::melee);
  }
}
} // namespace

std::unique_ptr<Sidekick> SidekickFactory::create(const std::string &path) {
  auto sidekick = std::make_unique<Sidekick>();
  nlohmann::json jSidekick = load(path);
  setStats(jSidekick, sidekick.get());
  return sidekick;
}
