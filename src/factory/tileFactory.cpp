#include "factory/tileFactory.h"
#include "libraries/magic_enum.hpp"
#include <cctype>
#include <string>


std::unique_ptr<Tile> TileFactory::create(const nlohmann::json &tile) {
  auto t = std::make_unique<Tile>();
  t->setId(tile["id"]);

  if (tile.contains("neighbors")) {
    for (const auto &neigh : tile["neighbors"]) {
      t->addNeighbor(neigh);
    }
  }

  if (tile.contains("tags")) {
    for (const auto &tg : tile["tags"]) {
      std::string tag = tg.get<std::string>();
      auto parsed = magic_enum::enum_cast<TypeOfTile>(tag);
      if (!parsed.has_value()) {
        parsed = magic_enum::enum_cast<TypeOfTile>(tag);
      }
      if (parsed.has_value()) {
        t->addTag(parsed.value());
      }
    }
  }

  if (tile.contains("zones")) {
    for (const auto &zn : tile["zones"]) {
      t->addZone(zn.get<std::string>());
    }
  }

  if (tile.contains("position")) {
    const auto &pos = tile["position"];
    Vector2D p;
    p.x = pos.value("x", 0);
    p.y = pos.value("y", 0);
    t->setPosition(p);
  }

  return t;
}
