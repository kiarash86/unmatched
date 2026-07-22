#include "model/map.h"
#include "model/fighter.h"
#include <queue>
#include <unordered_set>
// setting name
void Map::setName(const std::string &newName) { name = newName; }
// get name
std::string Map::getName() const { return name; }
// adding tile to map
void Map::addTile(const int id, std::unique_ptr<Tile> tile) {
  tiles[id] = std::move(tile);
}
// get all tiles
const std::unordered_map<int, std::unique_ptr<Tile>> &Map::getTiles() const {
  return tiles;
}
// give id, get tile
Tile *Map::getTile(int id) const {
  auto it = tiles.find(id);
  return it == tiles.end() ? nullptr : it->second.get();
}
// put a fighter in tile
void Map::placeFighter(Fighter *fighter, int tileId) {
  removeFighter(fighter);
  occupancy[tileId] = fighter;
  fighterTile[fighter] = tileId;
  fighter->setTileId(tileId);
  if (Tile *t = getTile(tileId)) {
    fighter->setPosition(t->getPosition());
  }
}
// remove a figher in tile
void Map::removeFighter(Fighter *fighter) // erase the tile from fighters
{
  auto it = fighterTile.find(fighter);
  if (it != fighterTile.end()) {
    auto it2 = occupancy.find(it->second);
    if (it2 != occupancy.end() && it2->second == fighter) {
      occupancy.erase(it2);
    }
    fighterTile.erase(it);
  }
}
// give fighter, get position(tile)
int Map::getTileIdOf(Fighter *fighter) const {
  auto it = fighterTile.find(fighter);
  return it == fighterTile.end() ? -1 : it->second;
}
// is someone here?
bool Map::isOccupied(int tileId) const {
  return occupancy.find(tileId) != occupancy.end();
}
// ok we know here is occuputed but who is he?
Fighter *Map::getFighterAt(int tileId) const {
  auto it = occupancy.find(tileId);
  return it == occupancy.end() ? nullptr : it->second;
}
// we have fighters in map, so give your identity(player) and sidekick or hero?
// get vector(because of sisters)
std::vector<Fighter *> Map::getFighter(const TypeOfFighter &type,
                                       int player) const {
  std::vector<Fighter *> result;
  for (auto &[tileId, fighter] : occupancy) {
    if (fighter == nullptr) {
      continue;
    }
    if (fighter->getOwnerPlayer() != player) {
      continue;
    }
    if (fighter->getFighterType() != type) {
      continue;
    }
    result.push_back(fighter);
  }
  return result;
}
// tiles that can reach with a value(distance)
std::vector<Tile *> Map::getTilesThatCanMoveTo(int fromTileId,
                                               int distance) const {
  std::vector<Tile *> result;
  if (!getTile(fromTileId)) {
    return result;
  }

  std::unordered_map<int, int> depth;
  std::queue<int> que; // using queue for bfs
  depth[fromTileId] = 0;
  que.push(fromTileId);

  while (!que.empty()) {
    int current = que.front();
    que.pop();
    int currentDepth = depth[current];
    if (currentDepth >= distance) {
      continue;
    }

    Tile *tile = getTile(current);
    if (!tile) {
      continue;
    }

    for (int neighborId : tile->getNeighbors()) {
      if (depth.find(neighborId) != depth.end()) {
        continue;
      }
      depth[neighborId] = currentDepth + 1;
      que.push(neighborId);
    }
  }

  for (auto &[tileId, d] : depth) {
    if (tileId == fromTileId) {
      continue;
    }
    if (Tile *t = getTile(tileId)) {
      result.push_back(t);
    }
  }
  return result;
}
// give 2 id get distance
int Map::distanceBetween(int tileIdA, int tileIdB) const {
  if (tileIdA == tileIdB) {
    return 0;
  }
  if (!getTile(tileIdA) || !getTile(tileIdB)) {
    return -1;
  }

  std::unordered_map<int, int> depth;
  std::queue<int> que;
  depth[tileIdA] = 0;
  que.push(tileIdA);

  while (!que.empty()) {
    int current = que.front();
    que.pop();
    if (current == tileIdB) {
      return depth[current];
    }

    Tile *tile = getTile(current);
    if (!tile) {
      continue;
    }

    for (int neighborId : tile->getNeighbors()) {
      if (depth.find(neighborId) != depth.end()) {
        continue;
      }
      depth[neighborId] = depth[current] + 1;
      que.push(neighborId);
    }
  }
  return -1; // cant go there
}
// same zone get
std::vector<Tile *>
Map::getTilesInZones(const std::unordered_set<std::string> &zones) const {
  std::vector<Tile *> result;
  for (auto &[id, tile] : tiles) {
    for (const auto &zone : tile->getZones()) {
      if (zones.count(zone)) {
        result.push_back(tile.get());
        break;
      }
    }
  }
  return result;
}
// portals get for moving between them
std::vector<int> Map::getPortalTileIds() const {
  std::vector<int> c;
  for (auto &[id, tile] : tiles) {
    if (tile->isPortal()) {
      c.push_back(id);
    }
  }
  return c;
}
// not occuputed by others and...
std::vector<Tile *> Map::getReachableTiles(int fromTileId, int distance,
                                           const Fighter *mover) const {
  std::vector<Tile *> result;
  if (!getTile(fromTileId)) {
    return result;
  }

  int moverOwner = mover ? mover->getOwnerPlayer() : -1;

  std::unordered_map<int, int> depth;
  std::queue<int> que;
  depth[fromTileId] = 0;
  que.push(fromTileId);

  while (!que.empty()) {
    int current = que.front();
    que.pop();
    int currentDepth = depth[current];
    if (currentDepth >= distance) {
      continue;
    }

    if (current != fromTileId) {
      Fighter *fighter = getFighterAt(current);
      if (fighter && fighter->getOwnerPlayer() != moverOwner) {
        continue;
      }
    }

    Tile *tile = getTile(current);
    if (!tile) {

      continue;
    }

    for (int neighborId : tile->getNeighbors()) {
      if (depth.find(neighborId) != depth.end()) {
        continue;
      }
      depth[neighborId] = currentDepth + 1;
      que.push(neighborId);
    }

    if (tile->isPortal()) {
      for (int portalId : getPortalTileIds()) {
        if (portalId == current) {
          continue;
        }
        if (depth.find(portalId) != depth.end()) {
          continue;
        }
        depth[portalId] = currentDepth + 1;
        que.push(portalId);
      }
    }
  }

  for (auto &[tileId, d] : depth) {
    if (tileId == fromTileId) {
      continue;
    }
    if (isOccupied(tileId)) {
      continue;
    }
    if (Tile *t = getTile(tileId)) {
      result.push_back(t);
    }
  }
  return result;
}
// how much needed to go there
int Map::movementDistance(int fromTileId, int toTileId,
                          const Fighter *mover) const {
  if (fromTileId == toTileId) {
    return 0;
  }
  if (!getTile(fromTileId) || !getTile(toTileId)) {
    return -1;
  }

  int moverOwner = mover ? mover->getOwnerPlayer() : -1;

  std::unordered_map<int, int> depth;
  std::queue<int> que;
  depth[fromTileId] = 0;
  que.push(fromTileId);

  while (!que.empty()) {
    int current = que.front();
    que.pop();
    if (current == toTileId) {
      return depth[current];
    }

    if (current != fromTileId) {
      Fighter *occupant = getFighterAt(current);
      if (occupant && occupant->getOwnerPlayer() != moverOwner) {
        continue;
      }
    }

    Tile *tile = getTile(current);
    if (!tile) {
      continue;
    }

    for (int neighborId : tile->getNeighbors()) {
      if (depth.find(neighborId) != depth.end()) {
        continue;
      }
      depth[neighborId] = depth[current] + 1;
      que.push(neighborId);
    }

    if (tile->isPortal()) {
      for (int portalId : getPortalTileIds()) {
        if (portalId == current) {
          continue;
        }
        if (depth.find(portalId) != depth.end()) {
          continue;
        }
        depth[portalId] = depth[current] + 1;
        que.push(portalId);
      }
    }
  }
  return -1;
}

// change postion with another fighter
void Map::exchangePosition(Fighter *fighter1, Fighter *fighter2) {
  int tile1 = getTileIdOf(fighter1);
  int tile2 = getTileIdOf(fighter2);
  if (tile1 == -1 || tile2 == -1) {
    return;
  }
  placeFighter(fighter1, tile2);
  placeFighter(fighter2, tile1);
}
