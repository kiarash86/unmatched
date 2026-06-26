#include "../../include/factory/tileFactory.h"

std::unique_ptr<Tile> TileFactory::create(const nlohmann::json &tile) {

  auto t = std::make_unique<Tile>();
  t->setId(tile["id"]);
  for (const auto &neigh : tile["neighbors"]) {
    t->addNeighbor(neigh);
  }

  for (const auto &tg : tile["tags"]) {
//HACK : should checked with typeoftiles and convert from string to enum
    t->addTag(tg);
  }

  return t;
}
