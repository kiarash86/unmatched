  #include "model/tile.h"
  void Tile::setId(const int &ID)
  {
    id = ID;
  }
  int Tile::getId()
  {

      return id;
  }
  std::vector<int> Tile::getNeighbors()
  {
    return neighbors;
  }
  void Tile::setNeighbors(const std::vector<int> &nghbrs)
  { 
    neighbors = nghbrs;
  }
  void Tile::addNeighbor(const int &neighbor)
  {
    neighbors.push_back(neighbor);
  }
  void Tile::addTag(const TypeOfTile &typeTle)
  {
    tags.insert(typeTle);
  }
  std::unordered_set<TypeOfTile> Tile::getTags()
  {
    return tags;
  }
  void Tile::setTags(const std::unordered_set<TypeOfTile> &typeTiles)
  {
    tags = typeTiles;
  }