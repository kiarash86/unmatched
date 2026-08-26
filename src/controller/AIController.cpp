#include "controller/AIController.h"
#include "controller/GameManager.h"
#include "model/card.h"
#include "model/deck.h"
#include "model/fighter.h"
#include "model/hero.h"
#include "model/map.h"
#include "model/sidekick.h"
#include "model/tile.h"
#include "model/typeOfAttack.h"
#include "model/typeOfCard.h"
#include <climits>

AIController::AIController(int ownerPlayer) : ownerPlayer(ownerPlayer) {}

bool AIController::isControlling(const Hero *hero) const
{
  return hero && hero->getOwnerPlayer() == ownerPlayer;
}



bool AIController::pendingBelongsToMe(GameManager &gm) const
{

  if (gm.isAwaitingHeroStartPlacement())
  {
    Hero *h = gm.getHeroAwaitingStartPlacement();
    return h && isControlling(h);
  }
  if (gm.isAwaitingSidekickPlacement())
  {
    Fighter *sk = gm.getSidekickAwaitingPlacement();
    return sk && sk->getOwnerPlayer() == ownerPlayer;
  }
  if (gm.isAwaitingFogTokenPlacement())
  {
    Hero *h = gm.getFogTokenAwaitingHero();
    return h && isControlling(h);
  }
  if (Fighter *chooser = gm.getPendingChooser())
  {
    return chooser->getOwnerPlayer() == ownerPlayer;
  }
  if (gm.isCombatActive())
  {
    if (Hero *defender = gm.getCombatDefendingHero())
    {
      return isControlling(defender);
    }
  }
  Hero *current = gm.getCurrentHero();
  return current && isControlling(current);
}




void AIController::resolveTileChoice(GameManager &gm)
{
  std::vector<Tile *> options = gm.getValidTiles();
  if (options.empty())
  {
    return;
  }

  Fighter *reference = gm.getPendingChooser();
  if (!reference)
  {
    reference = gm.getCurrentHero();
  }

  Tile *best = options.front();
  if (reference)
  {
    if (Fighter *enemyRef = nearestEnemyFighter(gm, reference))
    {
      int bestDist = INT_MAX;
      for (Tile *t : options)
      {
        int d = gm.getMap().distanceBetween(t->getId(), enemyRef->getTileId());
        if (d >= 0 && d < bestDist)
        {
          bestDist = d;
          best = t;
        }
      }
    }
  }
  gm.submitTile(best);
}









void AIController::resolveFighterChoice(GameManager &gm)
{
  std::vector<Fighter *> options = gm.getValidFighters();
  if (options.empty())
  {
    return;
  }

  std::vector<Fighter *> enemies, allies;
  bool hasDecline = false;
  for (Fighter *f : options)
  {
    if (!f)
    {
      hasDecline = true;
      continue;
    }
    if (f->getOwnerPlayer() == ownerPlayer)
    {
      allies.push_back(f);
    }
    else
    {
      enemies.push_back(f);
    }
  }

  Fighter *choice = nullptr;
  if (!enemies.empty())
  {
    choice = weakestOf(enemies);
  }
  else if (!allies.empty())
  {
    choice = weakestOf(allies);
  }
  else if (hasDecline)
  {
    choice = nullptr;
  }
  gm.submitFighter(choice);
}



Fighter *AIController::weakestOf(const std::vector<Fighter *> &fighters)
{
  Fighter *best = nullptr;
  for (Fighter *f : fighters)
  {
    if (!f)
    {
      continue;
    }
    if (!best || f->getHealth() < best->getHealth())
    {
      best = f;
    }
  }
  return best;
}







bool AIController::inAttackRange(GameManager &gm, Fighter *attacker, Fighter *target) const
{
  Map &board = gm.getMap();
  int fromTile = attacker->getTileId();
  int toTile = target->getTileId();
  int distance = board.distanceBetween(fromTile, toTile);

  if (attacker->getTypeOfAttack() == TypeOfAttack::melee)
  {
    return distance == 1;
  }

  if (distance == 1)
  {
    return true;
  }
  Tile *fromT = board.getTile(fromTile);
  Tile *toT = board.getTile(toTile);
  if (!fromT || !toT)
  {
    return false;
  }
  for (const auto &zone : fromT->getZones())
  {
    if (toT->getZones().count(zone))
    {
      return true;
    }
  }
  return false;
}







Fighter *AIController::nearestEnemyFighter(GameManager &gm, Fighter *from) const
{
  if (!from)
  {
    return nullptr;
  }
  Fighter *best = nullptr;
  int bestDist = INT_MAX;
  for (int p = 0; p < gm.getPlayerCount(); p++)
  {
    Hero *h = gm.getHero(p);
    if (!h || h->getOwnerPlayer() == from->getOwnerPlayer())
    {
      continue;
    }
    std::vector<Fighter *> candidates;
    candidates.push_back(h);
    for (auto &sk : h->getSidekicks())
    {
      if (sk->isAlive())
      {
        candidates.push_back(sk.get());
      }
    }
    for (Fighter *f : candidates)
    {
      if (!f->isAlive())
      {
        continue;
      }
      int d = gm.getMap().distanceBetween(from->getTileId(), f->getTileId());
      if (d >= 0 && d < bestDist)
      {
        bestDist = d;
        best = f;
      }
    }
  }
  return best;
}



void AIController::resolvePendingSelection(GameManager &gm)
{
  if (!pendingBelongsToMe(gm))
  {
    return;
  }
  if (gm.isWaitingForTile())
  {
    resolveTileChoice(gm);
  }
  else if (gm.isWaitingForFighter())
  {
    resolveFighterChoice(gm);
  }
  else if (gm.isWaitingForCard())
  {
    resolveCardChoice(gm);
  }
  else if (gm.isWaitingForEffectChoice())
  {
    resolveEffectChoice(gm);
  }
}






























































































void AIController::resolveCardChoice(GameManager &gm)
{
  std::vector<Card *> options = gm.getValidCards();
  if (options.empty())
  {
    gm.submitCard(nullptr);
    return;
  }

  Card *best = options.front();
  int bestScore = INT_MIN;
  for (Card *c : options)
  {
    if (!c)
    {
      continue;
    }
    int score = c->getAttackStat() + c->getDefStat() + c->getBoost();
    if (score > bestScore)
    {
      bestScore = score;
      best = c;
    }
  }
  gm.submitCard(best);
}

void AIController::resolveEffectChoice(GameManager &gm)
{
  std::vector<std::string> labels = gm.getValidEffectChoiceLabels();
  if (labels.empty())
  {
    return;
  }
  gm.submitEffectChoice(0);
}

void AIController::resolveDefenseIfNeeded(GameManager &gm)
{
  Hero *defender = gm.getCombatDefendingHero();
  if (!defender || !isControlling(defender))
  {
    return;
  }

  Card *chosen = pickDefenseCard(gm, defender);

  int predicted = -1;
  if (chosen && chosen->needsPrediction())
  {
    if (Card *attackerCard = gm.getCombatAttackerCard())
    {
      predicted = attackerCard->getAttackStat();
    }
  }

  gm.resolveCombat(chosen, predicted);
}

Card *AIController::pickDefenseCard(GameManager &gm, Hero *defender) const
{
  if (!defender || !defender->getDeck())
  {
    return nullptr;
  }
  Card *best = nullptr;
  int bestStat = -1;
  for (Card *c : defender->getDeck()->getHand())
  {
    if (c->getCardType() != TypeOfCard::def &&
        c->getCardType() != TypeOfCard::multipurpose)
    {
      continue;
    }
    if (!gm.canPerform(c, defender))
    {
      continue;
    }
    if (c->getDefStat() > bestStat)
    {
      bestStat = c->getDefStat();
      best = c;
    }
  }
  return best;
}

void AIController::takeTurnStep(GameManager &gm)
{
  Hero *self = gm.getCurrentHero();
  if (!self)
  {
    return;
  }

  if (gm.getActionsRemaining() <= 0)
  {
    gm.endTurn();
    return;
  }

  Hero *enemy = nullptr;
  for (int p = 0; p < gm.getPlayerCount(); p++)
  {
    Hero *h = gm.getHero(p);
    if (h && !isControlling(h))
    {
      enemy = h;
      break;
    }
  }
  if (!enemy)
  {
    gm.endTurn();
    return;
  }

  if (tryAttack(gm, self, enemy))
  {
    return;
  }
  if (tryPlayUsefulCard(gm, self, enemy))
  {
    return;
  }
  if (tryMoveTowardEnemy(gm, self, enemy))
  {
    return;
  }

  if (gm.performManeuver())
  {
    gm.finishManeuver();
    return;
  }

  gm.endTurn();
}

bool AIController::tryAttack(GameManager &gm, Hero *self, Hero *enemy)
{
  if (gm.getActionsRemaining() <= 0 || !self->getDeck())
  {
    return false;
  }

  std::vector<Fighter *> attackers;
  attackers.push_back(self);
  for (auto &sk : self->getSidekicks())
  {
    if (sk->isAlive())
    {
      attackers.push_back(sk.get());
    }
  }

  std::vector<Fighter *> targets;
  targets.push_back(enemy);
  for (auto &sk : enemy->getSidekicks())
  {
    if (sk->isAlive())
    {
      targets.push_back(sk.get());
    }
  }

  Fighter *bestAttacker = nullptr;
  Card *bestCard = nullptr;
  Fighter *bestTarget = nullptr;
  int bestScore = INT_MIN;

  for (Fighter *attacker : attackers)
  {
    if (!attacker->isAlive())
    {
      continue;
    }
    for (Card *card : self->getDeck()->getHand())
    {
      if (card->getCardType() != TypeOfCard::attack &&
          card->getCardType() != TypeOfCard::multipurpose)
      {
        continue;
      }
      if (!gm.canPerform(card, attacker))
      {
        continue;
      }
      for (Fighter *target : targets)
      {
        if (!target->isAlive() || !inAttackRange(gm, attacker, target))
        {
          continue;
        }

        int score = card->getAttackStat() * 10 - target->getHealth();
        if (score > bestScore)
        {
          bestScore = score;
          bestAttacker = attacker;
          bestCard = card;
          bestTarget = target;
        }
      }
    }
  }

  if (!bestAttacker)
  {
    return false;
  }
  return gm.startCombat(bestCard, bestAttacker, bestTarget);
}

bool AIController::tryPlayUsefulCard(GameManager &gm, Hero *self, Hero *enemy)
{
  (void)enemy;
  if (gm.getActionsRemaining() <= 0 || !self->getDeck())
  {
    return false;
  }

  for (Card *card : self->getDeck()->getHand())
  {
    if (card->getCardType() != TypeOfCard::event)
    {
      continue;
    }
    if (!gm.canPerform(card, self))
    {
      continue;
    }

    if (!gm.cardNeedsTarget(card))
    {
      if (gm.playCard(card))
      {
        return true;
      }
      continue;
    }

    std::vector<Fighter *> targets = gm.getValidTargetsForCard(card);
    if (targets.empty())
    {
      continue;
    }

    Fighter *target = nullptr;
    if (card->isTargetsAnyFighter())
    {
      std::vector<Fighter *> allies;
      for (Fighter *f : targets)
      {
        if (f->getOwnerPlayer() == ownerPlayer)
        {
          allies.push_back(f);
        }
      }
      target = !allies.empty() ? weakestOf(allies) : weakestOf(targets);
    }
    else
    {
      target = weakestOf(targets);
    }
    if (!target)
    {
      continue;
    }

    Hero *enemyHero = gm.getHero(target->getOwnerPlayer());
    if (!enemyHero)
    {
      enemyHero = target->getOwnerPlayer() == self->getOwnerPlayer() ? self : nullptr;
    }
    if (gm.playCard(card, self, target, enemyHero))
    {
      return true;
    }
  }
  return false;
}

bool AIController::tryMoveTowardEnemy(GameManager &gm, Hero *self, Hero *enemy)
{
  (void)enemy;
  if (gm.getActionsRemaining() <= 0)
  {
    return false;
  }

  Fighter *mover = self;
  Fighter *nearest = nearestEnemyFighter(gm, mover);
  if (!nearest)
  {
    return false;
  }

  int currentDistance = gm.getMap().distanceBetween(mover->getTileId(), nearest->getTileId());
  if (currentDistance >= 0 && currentDistance <= 1)
  {
    return false;
  }

  if (!gm.performManeuver())
  {
    return false;
  }

  Map &board = gm.getMap();
  std::vector<Tile *> reachable =
      board.getReachableTiles(mover->getTileId(), gm.getMovesRemaining(mover), mover);

  Tile *best = nullptr;
  int bestDist = currentDistance;
  for (Tile *t : reachable)
  {
    int d = board.distanceBetween(t->getId(), nearest->getTileId());
    if (d >= 0 && d < bestDist)
    {
      bestDist = d;
      best = t;
    }
  }
  if (best)
  {
    gm.moveFighter(mover, best->getId());
  }
  gm.finishManeuver();
  return true;
}

void AIController::discardOne(GameManager &gm, Hero *self)
{
  if (!self->getDeck())
  {
    return;
  }
  std::vector<Card *> hand = self->getDeck()->getHand();
  if (hand.empty())
  {
    return;
  }

  Card *worst = hand.front();
  int worstScore = INT_MAX;
  for (Card *c : hand)
  {
    int score = c->getAttackStat() + c->getDefStat() + c->getBoost();
    if (score < worstScore)
    {
      worstScore = score;
      worst = c;
    }
  }
  gm.discardExcessCard(worst);
}



void AIController::update(GameManager &gm)
{
  if (gm.isGameOver())
  {
    return;
  }

  if (gm.isWaitingForSelection())
  {
    resolvePendingSelection(gm);
    return;
  }

  if (gm.isCombatActive())
  {
    resolveDefenseIfNeeded(gm);
    return;
  }

  Hero *current = gm.getCurrentHero();
  if (!current || !isControlling(current))
  {
    return;
  }

  if (gm.needsEndOfTurnDiscard())
  {
    discardOne(gm, current);
    return;
  }

  takeTurnStep(gm);
}