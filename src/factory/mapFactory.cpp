#include "factory/mapFactory.h"

std::unique_ptr<Map> MapFactory::create(const std::string &nameMap) {
  std::string path = "data/maps/" + nameMap + ".json";
  nlohmann::json mp = load(path);

  auto m = std::make_unique<Map>();
  m->setName(mp["name"]);
  for (const auto &t : mp["tiles"]) {
    int id = t["id"];
    m->addTile(id, TileFactory::create(t));
  }

  return m;
}
