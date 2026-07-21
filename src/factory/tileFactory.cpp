#include "factory/tileFactory.h"
#include "libraries/magic_enum.hpp"
#include "utility/exceptions.h"
#include <cctype>
#include <string>


std::unique_ptr<Tile> TileFactory::create(const nlohmann::json &tile) {
  try {
  auto t = std::make_unique<Tile>();
  t->setId(json_util::requireInt(tile, "id", "Tile"));

  if (tile.contains("neighbors")) {
    for (const auto &neigh : tile["neighbors"]) {
      t->addNeighbor(neigh);
    }
  }

  if (tile.contains("tags")) {
    for (const auto &tg : tile["tags"]) {
      std::string tag = tg.get<std::string>();
      auto parsed = magic_enum::enum_cast<TypeOfTile>(tag);
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
  } catch (const AppException &e) {
    throw FactoryException(std::string("Failed to build Tile: ") + e.what());
  } catch (const nlohmann::json::exception &e) {
    throw FactoryException(std::string("Failed to build Tile: malformed data (") +
                            e.what() + ")");
  }
}
