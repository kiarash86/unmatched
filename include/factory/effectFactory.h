#pragma once
#include <memory>
#include <nlohmann/json.hpp>
#include "utility/file.h"
#include "engine/effects/effect.h"
#include "engine/effects/dmgEffect.h"
#include "engine/effects/drawEffect.h"
#include "engine/effects/moveEffect.h"
#include "engine/effects/removeCardEffect.h"
#include "engine/effects/removeEffectEffect.h"
#include "engine/effects/seeHandEffect.h"
#include "engine/effects/positionExchangeEffect.h"
#include "engine/effects/addEffect.h"
#include "engine/effects/changeValueEffect.h"
#include "engine/effects/choosePlaceEffect.h"
#include "engine/effects/reviveEffect.h"
#include "engine/effects/predictEffect.h"
#include "factory/conditionFactory.h"

class EffectFactory {
public:
  static std::unique_ptr<Effect> create(const nlohmann::json &effect);
};
