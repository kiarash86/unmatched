#include "../../include/factory/heroFactory.h"

// get hero with stats
// put data in the hero
static void setStats(const nlohmann::json &stats, Hero *hero)
{

    // setting data in hero
    hero->setName(stats["name"].get<std::string>());
    hero->setImgSource(stats["img"].get<std::string>());
    hero->setHealth(stats["health"].get<int>());
    hero->setMovement(stats["movement"].get<int>());
    if (stats["attackType"] == "range")
        hero->setTypeOfAttack(TypeOfAttack::ranged);
    else if (stats["attackType"] == "melee")
        hero->setTypeOfAttack(TypeOfAttack::melee);
}

// get the name 
// connect it to data/ + "name"
// with this we decide whick hero we are talking about
// make hero
// put json in stats and send it to setStats
// get the list of sidekicks from stats in a forrange
// we send the path to sidekickFactory and adding that to the hero
// samething for ability and deck
// return hero
std::unique_ptr<Hero> HeroFactory::create(const std::string &name)
{

    // path of hero
    std::string path = "data/" + name;

    // the hero we return it
    auto hero = std::make_unique<Hero>();

    // stats of hero will be in this
    nlohmann::json stats;

    // using load from utility to get json
    stats = load(path + "/stats.json");

    // put stats in hero
    setStats(stats, hero.get());

    // put sidekicks in hero
    for (auto &&Sidek : stats["sidekickList"])
    {
        // calling sidekickFactory to create the sidekick
        hero->addSidekick(SidekickFactory::create(path + "/Sidekick/" + Sidek.get<std::string>() + ".json"));
    }

    // calling abilityFactory to create ability
    hero->setAbility(AbilityFactory::create(path + "/abilities"));

    // calling deckFactory to create deck
    hero->setDeck(DeckFactory::create(path + "/deck"));

    return hero;
}