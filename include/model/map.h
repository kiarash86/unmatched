#include<vector>
#include <memory>
#include "tile.h"
#include <unordered_map>
class Map
{
private:
    std::string name;
    std::unordered_map< int , std::unique_ptr< Tile>> tiles;
public:
    void setName(const std::string &newName);
    std::string getName();
    void setTiles(const std::unordered_map<int ,  std::unique_ptr<Tile>> &NewTiles);
    void addTile(const int id, const  std::unique_ptr<Tile> & tle);
    std::unordered_map<int ,  std::unique_ptr<Tile>> getTiles();

};

