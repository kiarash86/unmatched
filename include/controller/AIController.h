#pragma once
#include <vector>

class GameManager;
class Hero;
class Fighter;
class Card;
class Tile;


class AIController {
public:
  explicit AIController(int ownerPlayer);


  void update(GameManager &gm);

  bool isControlling(const Hero *hero) const;

  int getOwnerPlayer() const { return ownerPlayer; }

private:
  int ownerPlayer;


  bool pendingBelongsToMe(GameManager &gm) const;

  void resolvePendingSelection(GameManager &gm);
  void resolveTileChoice(GameManager &gm);
  void resolveFighterChoice(GameManager &gm);
  void resolveCardChoice(GameManager &gm);
  void resolveEffectChoice(GameManager &gm);

  void resolveDefenseIfNeeded(GameManager &gm);
  Card *pickDefenseCard(GameManager &gm, Hero *defender) const;

  void takeTurnStep(GameManager &gm);
  bool tryAttack(GameManager &gm, Hero *self, Hero *enemy);
  bool tryPlayUsefulCard(GameManager &gm, Hero *self, Hero *enemy);
  bool tryMoveTowardEnemy(GameManager &gm, Hero *self, Hero *enemy);
  void discardOne(GameManager &gm, Hero *self);

  Fighter *nearestEnemyFighter(GameManager &gm, Fighter *from) const;
  bool inAttackRange(GameManager &gm, Fighter *attacker, Fighter *target) const;
  static Fighter *weakestOf(const std::vector<Fighter *> &fighters);
};
