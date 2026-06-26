#include "model/map.h"

void Map::setName(const std::string &newName) { name = newName; }
std::string Map::getName() { return name; }
void Map::setTiles(
    const std::unordered_map<int, std::unique_ptr<Tile>> &NewTiles) {
  // FIXME: typeo
  tiles = NewTiles;
}
void Map::addTile(const int id, const std::unique_ptr<Tile> &tle) {
  tiles.insert_or_assign(id, tle);
}
std::unordered_map<int, std::unique_ptr<Tile>> Map::getTiles() { return tiles; }