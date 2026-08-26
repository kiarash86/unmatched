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
