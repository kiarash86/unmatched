#include <memory>
#include "../model/hero.h"
#include <nlohmann/json.hpp>
#include "../utility/file.h"
#include"sidekickFactory.h"
#include"deckFactory.h"

class HeroFactory
{
public:
    static std::unique_ptr<Hero>& create(const std::string & name);
};
