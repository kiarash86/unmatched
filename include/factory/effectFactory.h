#include <memory>
#include <nlohmann/json.hpp>
#include "../utility/file.h"
#include "cardFactory.h"
#include "engine/effects/effect.h"
#include"engine/effects/modifierEffect.h"
#include "factory/conditionFactory.h"
class EffectFactory
{
private:

public:

static std::unique_ptr<Effect> create(const nlohmann::json & effect);

};