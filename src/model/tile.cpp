#include "model/tile.h"

void Tile::setId(const int &ID) { id = ID; }
int Tile::getId() const { return id; }
std::vector<int> Tile::getNeighbors() const { return neighbors; }
void Tile::setNeighbors(const std::vector<int> &nghbrs) { neighbors = nghbrs; }
void Tile::addNeighbor(const int &neighbor) { neighbors.push_back(neighbor); }
void Tile::addTag(const TypeOfTile &typeTle) { tags.insert(typeTle); }
std::unordered_set<TypeOfTile> Tile::getTags() const { return tags; }
void Tile::setTags(const std::unordered_set<TypeOfTile> &typeTiles) {
  tags = typeTiles;
}
bool Tile::isPortal() const { return tags.count(TypeOfTile::secretPassage) > 0; }

void Tile::setPosition(const Vector2D &pos) { position = pos; }
Vector2D Tile::getPosition() const { return position; }

void Tile::addZone(const std::string &zone) { zones.insert(zone); }
const std::unordered_set<std::string> &Tile::getZones() const { return zones; }
