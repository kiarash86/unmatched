#include "../../include/factory/cardFactory.h"


 std::unique_ptr<Card> CardFactory::create(const nlohmann::json & card)
{

    std::unique_ptr<Card> crd;
    
    crd->setName(card["name"]);
    crd->setImgSource(card["img"]);
    crd->setBoost(card["boost"]);
    crd->setPerformer(card["performer"]);
    crd->setCardType(card["type"]);
    crd->setEventType(card["eventType"]);
    for (const auto &eff : card["effects"])
    {
        
        crd->addEffect(EffectFactory::create(eff));
    }
    
    
    

    return crd;
}




// {
//     "name": "administer_aid", 
//     "img": "../../../../assets/images/sherlock/administer_aid.png",
//     "boost" : 2,
//     "performer":"watson", 
//     "type": "event",
//     "effects": [
//                 {
//             "type" :"add",
//             "toWhat": "health",
//             "howMuch": 1 , 
//             "who":"hero"

//         },
//         {
//             "type": "move",
//             "howMany": 1 ,
//             "whichOne":"sidekick",
//             "condition": "none",
//             "distance": -1,
//             "toWhere": "nearHero"

//         },
//          {
//             "type": "draw_card",
//             "howMany": 1 

//         }
//     ]

// }

