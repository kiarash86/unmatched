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










