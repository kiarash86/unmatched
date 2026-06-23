#include "../../include/factory/sidekickFactory.h"

// get hero with stats
// put data in the hero
namespace
{

    void setStats(const nlohmann::json &stats, Sidekick *sidekick)
    {

        // setting data in sidekick
        sidekick->setName(stats["name"].get<std::string>());
        sidekick->setImgSource(stats["img"].get<std::string>());
        sidekick->setHealth(stats["health"].get<int>());
        sidekick->setMovement(stats["movement"].get<int>());
        if (stats["attackType"].get<std::string>() == "range")
            sidekick->setTypeOfAttack(TypeOfAttack::ranged);
        else if (stats["attackType"].get<std::string>() == "melee")
            sidekick->setTypeOfAttack(TypeOfAttack::melee);
    }

}

std::unique_ptr<Sidekick>& SidekickFactory::create(const std::string & path)
{

    auto sidekick = std::make_unique<Sidekick>();

    nlohmann::json jSidekick = load(path);

    setStats(jSidekick, sidekick.get());

    return sidekick;
}
