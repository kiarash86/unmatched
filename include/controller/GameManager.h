#pragma once
#include "model/card.h"
#include "model/map.h"
#include <variant>
#include <vector>
class Player;

using PendingSelection =
    std::variant<std::monostate, std::vector<Tile *>, std::vector<Fighter *>,
                 std::vector<Card *>>;
class GameManager {
public:
  void requestTileChoice(std::vector<Tile *> options) {
    pending = std::move(options);
  }
  void requestFighterChoice(std::vector<Fighter *> options) {
    pending = std::move(options);
  }
  void requestCardChoice(std::vector<Card *> options) {
    pending = std::move(options);
  }

  bool isWaitingForTile() {
    return std::holds_alternative<std::vector<Tile *>>(pending);
  }

  bool isWaitingForFighter() {
    return std::holds_alternative<std::vector<Fighter *>>(pending);
  }
  bool isWaitingForCard() {
    return std::holds_alternative<std::vector<Card *>>(pending);
  }

  std::vector<Tile *> getValidTiles() {
    return std::get<std::vector<Tile *>>(pending);
  }
  std::vector<Fighter *> getValidFighters() {
    return std::get<std::vector<Fighter *>>(pending);
  }
  std::vector<Card *> getValidCards() {
    return std::get<std::vector<Card *>>(pending);
  }

  void submitTile(Tile *chosen) {
    if (!isWaitingForTile()) {
      return;
    }

    // TODO : what do u want to do here?
  }
  void submitFighter(Fighter *chosen) {
    if (!isWaitingForFighter()) {
      return;
    }

    // TODO : what do u want to do here?
  }
  void submitCard(Card *chosen) {
    if (!isWaitingForCard()) {
      return;
    }

    // TODO : what do u want to do here?
  }
  void clearPending() { pending = std::monostate{}; }

  private:
  std::vector<Player> players;
  int currentTurn{};
  int moves{};
  Map map;
  PendingSelection pending;

  Map &getMap() { return map; }

  void gameLoop();
  // FIXME: here maps should be controlled with enums
  GameManager(Map map) : map(map){};
  ~GameManager();
};
