#pragma once
#include "utility/file.h"
#include <memory>
#include "model/InfoHero.h"
#include "model/heroList.h"
#include "libraries/magic_enum.hpp"
class HeroInfoFactory 
{
private:
public:
    static std::unique_ptr<InfoHero> create(const HeroList & name);
};

