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
