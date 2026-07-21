#pragma once
#include "typeOfTile.h"
#include <string>
#include <unordered_set>
#include <vector>

struct Vector2D {
  int x{};
  int y{};
  bool operator==(const Vector2D &other) const {
    return x == other.x && y == other.y;
  }
};

class Tile {
private:
  int id{};
  std::vector<int> neighbors;
  std::unordered_set<TypeOfTile> tags;
  std::unordered_set<std::string> zones;
  Vector2D position;

public:
  void setId(const int &ID);
  int getId() const;
  std::vector<int> getNeighbors() const;
  void setNeighbors(const std::vector<int> &nghbrs);
  void addNeighbor(const int &neighbor);
  void addTag(const TypeOfTile &typeTle);
  std::unordered_set<TypeOfTile> getTags() const;
  void setTags(const std::unordered_set<TypeOfTile> &typeTiles);
  bool isPortal() const;

  void setPosition(const Vector2D &pos);
  Vector2D getPosition() const;

  void addZone(const std::string &zone);
  const std::unordered_set<std::string> &getZones() const;
};
