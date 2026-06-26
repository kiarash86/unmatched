#include "typeOfTile.h"
#include <string>
#include <unordered_set>
#include <vector>
class Tile {
private:
  int id;
  std::vector<int> neighbors;
  std::unordered_set<TypeOfTile> tags;
  std::unordered_set<std::string> zones;

public:
  void setId(const int &ID);
  int getId();
  std::vector<int> getNeighbors();
  void setNeighbors(const std::vector<int> &nghbrs);
  void addNeighbor(const int &neighbor);
  void addTag(const TypeOfTile &typeTle);
  std::unordered_set<TypeOfTile> getTags();
  void setTags(const std::unordered_set<TypeOfTile> &typeTiles);
};
