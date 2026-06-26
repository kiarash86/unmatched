#include <memory>
#include <nlohmann/json.hpp>
#include"model/tile.h"
class TileFactory
{
private:

public:

static std::unique_ptr<Tile> create(const nlohmann::json & tile);

};