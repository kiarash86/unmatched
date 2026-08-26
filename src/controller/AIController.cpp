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








