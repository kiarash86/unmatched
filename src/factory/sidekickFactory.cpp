#include "../../include/factory/sidekickFactory.h"


 std::unique_ptr<Sidekick> SidekickFactory::create(const nlohmann::json & sidekick)
{

    std::unique_ptr<Sidekick> sidekck;
    sidekck->setName(sidekick["name"]);
    sidekck->setImgSource(sidekick["img"]);
    sidekck->setHealth(sidekick["health"]);
    sidekck->setTypeOfAttack(sidekick["attackType"]);
    sidekck->setMovement(sidekick["movement"]);


    return sidekck;
}


// {
//     "name" : "watson" , 
//     "img": "../../../../assets/images/sherlock/watson.png",
//     "health" : 8,
//     "attackType": "ranged"  ,
//     "movement" : 2
// }