#include "../../include/factory/effectFactory.h"




 std::unique_ptr<Effect> EffectFactory::create(const nlohmann::json & effect)
 {
    auto eff = std::make_unique<Effect>;
    if (effect["condition"] != "none")
    {
        eff->addCondtion(ConditionFactory::create(effect["condtion"]));
    }
    
    
 }

