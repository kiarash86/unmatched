#pragma once

class Hero;
class Fighter;
class Card;

class IGameObserver {
public:
  virtual ~IGameObserver() = default;

  virtual void onGameStarted() {}
  virtual void onTurnEnded(int newCurrentPlayer) {}
  virtual void onCardPlayed(Hero *player, Card *card) {}
  
  virtual void onCardDiscarded(Hero *player, Card *card) {}
  virtual void onFighterMoved(Fighter *fighter, int fromTileId, int toTileId) {}
  virtual void onFighterDamaged(Fighter *fighter, int amount) {}
  
  virtual void onCombatResolved(Fighter *attacker, Fighter *defender,
                                Fighter *winner, Fighter *loser, int damage) {}
 
  virtual void onManeuverPerformed(Hero *hero, bool deckWasEmpty) {}
  
  virtual void onMovementBoosted(Hero *hero, Card *card, int gain) {}
 
  virtual void onFighterRemoved(Fighter *fighter) {}
 
  virtual void onGameOver(Hero *winner) {}
};
