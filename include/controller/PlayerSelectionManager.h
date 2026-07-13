#pragma once                // once per program
#include "model/Player.h"   // saving in player
#include "model/heroList.h" // list of heroes
#include <vector>           //vector

class PlayerSelectionManager // managing heroes
{
private:
  std::vector<Player> players; // all players

  PlayerSelectionManager() = default; // singelton class

public:
  static PlayerSelectionManager &instance() // return itself
  {
    static PlayerSelectionManager inst; // create one if there isnt any
    return inst;                        // return itself
  }

  void reset() // start from 0(no heroes for any players)
  {
    players.clear();
  }

  void addPlayer(const Player &player) // adding player
  {
    players.push_back(player);
  }

  bool
  isHeroTaken(HeroList hero) const // see if someone take this hero before or no
  {
    for (const auto &p : players) {
      if (p.getHeroId() == hero)
        return true;
    }
    return false;
  }

  int playerCount() const // number of players we have now
  {
    return (int)players.size();
  }

  const std::vector<Player> &getPlayers() const // get players
  {
    return players;
  }
};
