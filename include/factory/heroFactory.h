#include <memory>
#include "../model/hero.h"
#include <nlohmann/json.hpp>
#include "../utility/file.h"
class HeroFactory
{
private:




public:

static std::unique_ptr<Hero> create(const std::string & name);

};
