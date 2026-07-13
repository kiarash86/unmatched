#include<vector>
#include <memory>
#include "tile.h"
#include <unordered_map>
#include "model/typeOfFighter.h"
#include "model/fighter.h"
class Map
{
private:
    std::string name;
    std::unordered_map< int , std::unique_ptr< Tile>> tiles;
    std::unordered_map< Fighter , std::unique_ptr< Tile>> positions;
public:
    void setName(const std::string &newName);
    std::string getName();
    void setTiles(const std::unordered_map<int ,  std::unique_ptr<Tile>> &NewTiles);
    void addTile(const int id, const  std::unique_ptr<Tile> & tle);
    std::unordered_map<int ,  std::unique_ptr<Tile>> getTiles();
    std::vector<std::unique_ptr<Tile>> getPositionOfFighter(const TypeOfFighter & type , int player); //get type, return all fighter with that type
    std::vector<std::unique_ptr<Tile>> getTilesThatCanMoveTo(Tile tile , int distance);
    std::vector<std::unique_ptr<Tile>> getStartTile(Tile tile , int distance);
    std::vector<std::unique_ptr<Tile>> getTileWithZoneFilter(Tile tile , std::string zone);
    bool isOccupied(Tile tile);
    void exchangePosition(Fighter fighter1 , Fighter fighter2);

};

