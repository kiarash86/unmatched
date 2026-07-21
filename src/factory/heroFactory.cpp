#include "factory/heroFactory.h"

namespace {
void setStats(const nlohmann::json &stats, Hero *hero) {
  hero->setName(stats.value("name", ""));
  hero->setImgSource(stats.value("img", ""));
  hero->setHealth(stats.value("health", 1));
  hero->setMaxHealth(stats.value("health", 1));
  hero->setMovement(stats.value("movement", 0));
  std::string attackType = stats.value("attackType", "melee");
  if (attackType == "range") {
    hero->setTypeOfAttack(TypeOfAttack::ranged);
  } else {
    hero->setTypeOfAttack(TypeOfAttack::melee);
  }
}
}

std::unique_ptr<Hero> HeroFactory::create(const std::string &name) {
  std::string path = "data/" + name;

  auto hero = std::make_unique<Hero>();

  nlohmann::json stats = load(path + "/stats.json");
  setStats(stats, hero.get());

  if (stats.contains("sidekickList")) {
    for (auto &&sidekickName : stats["sidekickList"]) {
      std::string sidekickPath =
          path + "/sidekicks/" + sidekickName.get<std::string>() + ".json";
      hero->addSidekick(SidekickFactory::create(sidekickPath));
    }
  }

  hero->setAbility(AbilityFactory::create(path + "/abilities"));
  hero->setDeck(DeckFactory::create(path + "/deck"));

  return hero;
}
