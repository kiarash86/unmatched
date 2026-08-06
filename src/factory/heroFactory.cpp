#include "factory/heroFactory.h"
#include "utility/exceptions.h"

namespace {
void setStats(const nlohmann::json &stats, Hero *hero) {
  hero->setName(stats.value("name", ""));
  hero->setImgSource(stats.value("img", ""));
  hero->setHealth(stats.value("health", 1));
  hero->setMaxHealth(stats.value("health", 1));
  hero->setMovement(stats.value("movement", 0));
  std::string attackType = stats.value("attackType", "melee");

  if (attackType == "ranged") {
    hero->setTypeOfAttack(TypeOfAttack::ranged);
  } else {
    hero->setTypeOfAttack(TypeOfAttack::melee);
  }
}
}

std::unique_ptr<Hero> HeroFactory::create(const std::string &name) {
  std::string path = "data/" + name;

  try {
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
    if (stats.contains("fogTokenList") && stats["fogTokenList"].is_array()) {
      hero->setStartingFogTokenCount((int)stats["fogTokenList"].size());
    }
    hero->setDeck(DeckFactory::create(path + "/deck"));

    return hero;
  } catch (const AppException &e) {
    throw FactoryException("Failed to build Hero '" + name + "': " + e.what());
  } catch (const nlohmann::json::exception &e) {
    throw FactoryException("Failed to build Hero '" + name + "': malformed data (" +
                            e.what() + ")");
  }
}
