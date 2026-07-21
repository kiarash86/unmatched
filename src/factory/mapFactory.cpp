#include "factory/mapFactory.h"
#include "utility/exceptions.h"

std::unique_ptr<Map> MapFactory::create(const std::string &nameMap) {
  std::string path = "data/maps/" + nameMap + ".json";
  std::string context = "Map '" + nameMap + "' (" + path + ")";

  try {
    nlohmann::json mp = load(path);

    auto m = std::make_unique<Map>();
    m->setName(json_util::requireString(mp, "name", context));
    const auto &tiles = json_util::requireField(mp, "tiles", context);
    for (const auto &t : tiles) {
      int id = json_util::requireInt(t, "id", context + ", a tile");
      m->addTile(id, TileFactory::create(t));
    }

    return m;
  } catch (const AppException &e) {
    throw FactoryException("Failed to build " + context + ": " + e.what());
  } catch (const nlohmann::json::exception &e) {
    throw FactoryException("Failed to build " + context + ": malformed data (" +
                            e.what() + ")");
  }
}
