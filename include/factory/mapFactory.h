#include <memory>
#include <nlohmann/json.hpp>
#include "model/map.h"
#include "factory/tileFactory.h"
#include  "utility/file.h"
class MapFactory
{
private:

public:

static std::unique_ptr<Map> create(const std::string &nameMap);

};