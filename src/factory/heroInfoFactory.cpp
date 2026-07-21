#include "factory/heroInfoFactory.h"
#include "utility/exceptions.h"

std::unique_ptr<InfoHero> HeroInfoFactory::create(const HeroList &name) {
  std::string heroName = (std::string)magic_enum::enum_name(name);
  std::string path = "data/" + heroName + "/view.json";
  std::string context = "HeroInfo '" + heroName + "' (" + path + ")";

  try {
    auto heroInfo = load(path);
    auto hero = std::make_unique<InfoHero>();

    hero->name = json_util::requireString(heroInfo, "name", context);
    hero->role = json_util::requireString(heroInfo, "role", context);
    hero->hp = json_util::requireInt(heroInfo, "hp", context);
    hero->defense = json_util::requireInt(heroInfo, "defense", context);
    hero->attack = json_util::requireInt(heroInfo, "attack", context);
    hero->difficulty = json_util::requireInt(heroInfo, "difficulty", context);
    hero->speed = json_util::requireInt(heroInfo, "speed", context);
    hero->desc = json_util::requireString(heroInfo, "desc", context);
    hero->abilityDesc = json_util::requireString(heroInfo, "abilityDesc", context);
    hero->abilityTitle = json_util::requireString(heroInfo, "abilityTitle", context);

    const auto &clr = json_util::requireField(heroInfo, "themeColor", context);
    if (!clr.is_array() || clr.size() < 3) {
      throw DataFormatException(context + ": field 'themeColor' must be an array of 3 numbers");
    }
    hero->themeColor = Color{
        (unsigned char)clr[0],
        (unsigned char)clr[1],
        (unsigned char)clr[2],
        255};

    return hero;
  } catch (const AppException &e) {
    throw FactoryException("Failed to build " + context + ": " + e.what());
  } catch (const nlohmann::json::exception &e) {
    throw FactoryException("Failed to build " + context + ": malformed data (" +
                            e.what() + ")");
  }
}
