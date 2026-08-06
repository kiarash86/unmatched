#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "tile.h"
#include "model/typeOfFighter.h"

class Fighter;

class Map {
private:
  std::string name;
  std::unordered_map<int, std::unique_ptr<Tile>> tiles;
  // tile id -> the fighter 
  std::unordered_map<int, Fighter *> occupancy;
  // fighter -> tile id
  std::unordered_map<Fighter *, int> fighterTile;

public:
  void setName(const std::string &newName);
  std::string getName() const;

  void addTile(const int id, std::unique_ptr<Tile> tile);
  const std::unordered_map<int, std::unique_ptr<Tile>> &getTiles() const;
  Tile *getTile(int id) const;

 
  void placeFighter(Fighter *fighter, int tileId);
  void removeFighter(Fighter *fighter);
  int getTileIdOf(Fighter *fighter) const;
  bool isOccupied(int tileId) const;
  Fighter *getFighterAt(int tileId) const;


  std::vector<Fighter *> getFighter(const TypeOfFighter &type, int player) const;


  std::vector<Tile *> getTilesThatCanMoveTo(int fromTileId, int distance) const;

  
  int distanceBetween(int tileIdA, int tileIdB) const;

  
  std::vector<Tile *> getTilesInZones(const std::unordered_set<std::string> &zones) const;

 
  std::vector<Tile *> getReachableTiles(int fromTileId, int distance, const Fighter *mover) const;


  int movementDistance(int fromTileId, int toTileId, const Fighter *mover) const;

  void exchangePosition(Fighter *fighter1, Fighter *fighter2);

  void addFogToken(int tileId);

  bool removeFogToken(int tileId);

  bool hasFogToken(int tileId) const;

  int fogTokenCountAt(int tileId) const;

  std::vector<int> getFogTokenTileIds() const;

  void moveFogToken(int fromTileId, int toTileId);

private:
 
  std::vector<int> getPortalTileIds() const;

  std::unordered_map<int, int> fogTokensByTile;
};
