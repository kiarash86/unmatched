#include "../../include/factory/heroFactory.h"


 std::unique_ptr<Hero> HeroFactory::create(const std::string & name)
{
    std::unique_ptr<Hero> hero;
    nlohmann::json stats;
    if (name == "dracula")
    {
     

   stats = load("data/dracula/stats.h");
        hero->setName(stats["name"]);
        hero->setImgSource(stats["img"]);
        hero->setHealth(stats["health"]);
        hero->setMovement(stats["movement"]);
        hero->setTypeOfAttack(stats["attackType"]);
//here we have some repitive code and needed funcs

for (size_t i = 0; i < 3; i++)
{
    hero->addSidekick(SidekickFactory("data/dracula/Sidekick/sisters.json"));
    
}

        hero->setAbility(AbilityFactory("data/dracula/abilities"));
        hero-> setDeck(DeckFactory("data/dracula/deck"));
        
    }
    else if (name== "sherlock")
    {
        

   stats = load("data/sherlock/stats.h");
        hero->setName(stats["name"]);
        hero->setImgSource(stats["img"]);
        hero->setHealth(stats["health"]);
        hero->setMovement(stats["movement"]);
        hero->setTypeOfAttack(stats["attackType"]);
//here we have some repitive code and needed funcs

        hero->addSidekick(SidekickFactory("data/sherlock/Sidekick/watson.json"));
        hero->setAbility(AbilityFactory("data/sherlock/abilities"));
        hero-> setDeck(DeckFactory("data/sherlock/deck"));
        
    }
    return hero;
}




