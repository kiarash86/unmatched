#include "controller/GameManager.h"
#include "controller/PlayerSelectionManager.h"
#include "factory/heroFactory.h"
#include "factory/mapFactory.h"
#include "model/sidekick.h"
#include "model/deck.h"
#include "model/typeOfTile.h"
#include "engine/effects/effect.h"
#include "libraries/magic_enum.hpp"
#include "utility/exceptions.h"
#include <algorithm>
#include <filesystem>

namespace {

bool isDracula(const Fighter *fighter) {
  return fighter && fighter->getName() == "Dracula";
}
bool isSherlockOrWatson(const Fighter *fighter) {
  return fighter && (fighter->getName() == "SherlockHolms" ||
                      fighter->getName() == "watson");
}
bool isInvisibleMan(const Fighter *fighter) {
  return fighter && fighter->getName() == "InvisibleMan";
}
}

GameManager::GameManager(std::vector<std::unique_ptr<Hero>> heroesIn, std::unique_ptr<Map> mapIn)
    : heroes(std::move(heroesIn)), map(std::move(mapIn)) {
  for (int i = 0; i < (int)heroes.size(); i++) {
    heroes[i]->setOwnerPlayer(i);
  }
}

GameManager::~GameManager() = default;

std::unique_ptr<GameManager> GameManager::createFromSelection(const std::string &mapName) {
  try {
    std::vector<std::unique_ptr<Hero>> heroes;
    for (const auto &player : PlayerSelectionManager::instance().getPlayers()) {

      std::string folderName = std::string(magic_enum::enum_name(player.getHeroId()));
      heroes.push_back(HeroFactory::create(folderName));
    }

    auto map = MapFactory::create(mapName);
    return std::make_unique<GameManager>(std::move(heroes), std::move(map));
  } catch (const AppException &e) {

    throw FactoryException("Failed to set up match on map '" + mapName + "': " +
                            e.what());
  }
}

std::string GameManager::saveFilePath(int slot) {
  return "saves/save" + std::to_string(slot) + ".json";
}

bool GameManager::hasSave(int slot) {
  return std::filesystem::exists(saveFilePath(slot));
}

bool GameManager::saveGame(int slot) const {
(void)slot;
  return false;
}

std::unique_ptr<GameManager> GameManager::loadGame(int slot) {
(void)slot;
  return nullptr;
}

void GameManager::resetTurnState() {
  actionsRemaining = 2;
  movesRemainingByFighter.clear();
  maneuverActive = false;
  maneuverBoosted = false;
  clearPending();
  cancelCombat();

  if (Hero *hero = getCurrentHero()) {
    hero->setFatigued(false);
    for (auto &sk : hero->getSidekicks()) sk->setFatigued(false);
  }
}

void GameManager::startGame() {

  std::vector<int> startTileIds;
  for (auto &[id, tile] : map->getTiles()) {
    if (tile->getTags().count(TypeOfTile::startPoint)) {
      startTileIds.push_back(id);
    }
  }
  if (startTileIds.empty()) {
    for (auto &[id, tile] : map->getTiles()) {
      startTileIds.push_back(id);
    }
  }
  std::sort(startTileIds.begin(), startTileIds.end());
  allStartTileIds = startTileIds;

  placeHeroesFrom(0, startTileIds);
}

void GameManager::placeHeroesFrom(int heroIndex, std::vector<int> availableStartTileIds) {
  if (heroIndex >= (int)heroes.size()) {

    heroAwaitingStartPlacement = nullptr;
    placeSidekicksFrom(0, 0);
    return;
  }

  if (availableStartTileIds.empty()) {
    availableStartTileIds = allStartTileIds;
  }
  if (availableStartTileIds.empty()) {

    placeHeroesFrom(heroIndex + 1, availableStartTileIds);
    return;
  }

  Hero *hero = heroes[heroIndex].get();
  heroAwaitingStartPlacement = hero;

  std::vector<Tile *> options;
  for (int id : availableStartTileIds) {
    if (Tile *t = map->getTile(id)) {
      options.push_back(t);
    }
  }

  requestTileChoice(options, [this, heroIndex, availableStartTileIds](Tile *chosen) {
    Hero *hero = heroes[heroIndex].get();
    map->placeFighter(hero, chosen->getId());
    for (auto &sidekick : hero->getSidekicks()) {
      sidekick->setOwnerPlayer(heroIndex);
    }
  for (int i = 0; i < hero->getStartingFogTokenCount(); i++) {
      map->addFogToken(chosen->getId());
    }

    std::vector<int> remaining;
    for (int id : availableStartTileIds) {
      if (id != chosen->getId()) {
        remaining.push_back(id);
      }
    }
    placeHeroesFrom(heroIndex + 1, remaining);
  });
}

void GameManager::placeSidekicksFrom(int heroIndex, std::size_t sidekickIndex) {

  while (heroIndex < (int)heroes.size() &&
         sidekickIndex >= heroes[heroIndex]->getSidekicks().size()) {
    heroIndex++;
    sidekickIndex = 0;
  }

  if (heroIndex >= (int)heroes.size()) {

    sidekickAwaitingPlacement = nullptr;
    finishSetup();
    return;
  }

  Hero *hero = heroes[heroIndex].get();
  Fighter *sidekick = hero->getSidekicks()[sidekickIndex].get();
  sidekickAwaitingPlacement = sidekick;

  int heroTileId = map->getTileIdOf(hero);
  Tile *heroTile = map->getTile(heroTileId);
  std::vector<Tile *> options;
  if (heroTile) {
    for (Tile *t : map->getTilesInZones(heroTile->getZones())) {
      if (t->getId() == heroTileId || map->isOccupied(t->getId())) {
        continue;
      }
      options.push_back(t);
    }
    std::sort(options.begin(), options.end(),
              [](Tile *a, Tile *b) { return a->getId() < b->getId(); });
  }

  if (options.empty()) {

    placeSidekicksFrom(heroIndex, sidekickIndex + 1);
    return;
  }

  requestTileChoice(options, [this, heroIndex, sidekickIndex, sidekick](Tile *chosen) {
    map->placeFighter(sidekick, chosen->getId());
    placeSidekicksFrom(heroIndex, sidekickIndex + 1);
  });
}

void GameManager::finishSetup() {
  for (auto &hero : heroes) {
    if (hero->getDeck()) {
      hero->getDeck()->draw(5);
    }
  }

  currentTurn = 0;
  resetTurnState();
  triggerStartOfTurnAbility();

  for (auto *obs : observers) obs->onGameStarted();
}

int GameManager::getCurrentTurn() const { return currentTurn; }

Hero *GameManager::getCurrentHero() const {
  return getHero(currentTurn);
}

Hero *GameManager::getHero(int player) const {
  if (player < 0 || player >= (int)heroes.size()) {
    return nullptr;
  }
  return heroes[player].get();
}

int GameManager::getPlayerCount() const { return (int)heroes.size(); }
Map &GameManager::getMap() { return *map; }
int GameManager::getMovesRemaining(Fighter *fighter) const {
  if (!fighter) {
    return 0;
  }
  auto it = movesRemainingByFighter.find(fighter);
  if (it == movesRemainingByFighter.end()) {
    return 0;
  }
  return it->second;
}
int GameManager::getMovesRemaining() const { return getMovesRemaining(getCurrentHero()); }
int GameManager::getActionsRemaining() const { return actionsRemaining; }

void GameManager::addObserver(IGameObserver *observer) {
  if (observer) {
    observers.push_back(observer);
  }
}
void GameManager::removeObserver(IGameObserver *observer) {
  observers.erase(std::remove(observers.begin(), observers.end(), observer),
                  observers.end());
}

bool GameManager::isGameOver() const { return matchOver; }
Hero *GameManager::getWinner() const { return winningHero; }

void GameManager::checkForGameOver() {
  if (matchOver) {
    return;
  }

  Hero *lastStanding = nullptr;
  int aliveCount = 0;
  for (auto &hero : heroes) {
    if (hero && hero->isAlive()) {
      aliveCount++;
      lastStanding = hero.get();
    }
  }

  if (aliveCount == 1 && heroes.size() > 1) {
    matchOver = true;
    winningHero = lastStanding;
    for (auto *obs : observers) obs->onGameOver(winningHero);
  }
}

bool GameManager::endTurn() {
  if (heroes.empty() || matchOver) {
    return false;
  }
  if (actionsRemaining > 0) {
    return false;
  }
  if (needsEndOfTurnDiscard()) {
    return false;
  }

  currentTurn = (currentTurn + 1) % (int)heroes.size();
  resetTurnState();
  triggerStartOfTurnAbility();

  for (auto *obs : observers) obs->onTurnEnded(currentTurn);
  return true;
}

void GameManager::checkFighterDeath(Fighter *fighter) {
  if (!fighter || fighter->isAlive()) {
    return;
  }
  if (map->getTileIdOf(fighter) < 0) {
    return;
  }
  map->removeFighter(fighter);
  for (auto *obs : observers) obs->onFighterRemoved(fighter);
}

void GameManager::notifyDamage(Fighter *fighter, int healthBefore) {
  if (!fighter) {
    return;
  }
  int lost = healthBefore - fighter->getHealth();
  if (lost > 0) {
    for (auto *obs : observers) obs->onFighterDamaged(fighter, lost);
  }
  checkFighterDeath(fighter);
}

bool GameManager::heroHasCardInHand(const Hero *hero, const Card *card) const {
  if (!hero || !hero->getDeck() || !card) {
    return false;
  }
  auto hand = hero->getDeck()->getHand();
  return std::find(hand.begin(), hand.end(), card) != hand.end();
}

void GameManager::applyFatigue(Hero *hero) {
  if (!hero) {
    return;
  }

  auto hit = [&](Fighter *f) {
    if (!f || !f->isAlive()) {
      return;
    }
    f->setFatigued(true);
    int before = f->getHealth();
    f->damage(2);
    notifyDamage(f, before);
  };

  hit(hero);
  for (auto &sk : hero->getSidekicks()) hit(sk.get());
}

bool GameManager::performerAllows(const Card *card, const Fighter *actingFighter) const {
  if (!card || !actingFighter) {
    return false;
  }
  switch (card->getPerformer()) {
    case TypeOfPerformer::hero:
      return actingFighter->getFighterType() == TypeOfFighter::hero;
    case TypeOfPerformer::sidekick:
      return actingFighter->getFighterType() == TypeOfFighter::sidekick;
    case TypeOfPerformer::fighter:
    default:
      return true;
  }
}

bool GameManager::performManeuver() {
  if (matchOver || actionsRemaining <= 0) {
    return false;
  }

  Hero *hero = getCurrentHero();
  if (!hero || !hero->getDeck()) {
    return false;
  }

  Deck *deck = hero->getDeck();

  bool deckWasEmpty = deck->drawPileCount() == 0;
  if (deckWasEmpty) {
    applyFatigue(hero);
  } else {
    deck->draw(1);
  }

  actionsRemaining--;

  movesRemainingByFighter[hero] = hero->getMovement();
  for (auto &sk : hero->getSidekicks()) {
    if (sk->isAlive()) {
      movesRemainingByFighter[sk.get()] = sk->getMovement();
    }
  }
  maneuverActive = true;
  maneuverBoosted = false;

  for (auto *obs : observers) obs->onManeuverPerformed(hero, deckWasEmpty);
  checkForGameOver();
  return true;
}

bool GameManager::needsEndOfTurnDiscard() const {
  Hero *hero = getCurrentHero();
  return hero && hero->getDeck() && hero->getDeck()->handPileCount() > 7;
}

bool GameManager::discardExcessCard(Card *card) {
  if (!needsEndOfTurnDiscard() || !card) {
    return false;
  }

  Hero *hero = getCurrentHero();
  if (!heroHasCardInHand(hero, card)) {
    return false;
  }

  hero->getDeck()->discard(card);
  for (auto *obs : observers) obs->onCardDiscarded(hero, card);
  return true;
}

bool GameManager::moveFighter(Fighter *fighter, int tileId) {
  if (!fighter || matchOver) {
    return false;
  }
  int remaining = getMovesRemaining(fighter);
  if (remaining <= 0) {
    return false;
  }
  if (map->isOccupied(tileId)) {
    return false;
  }

  int fromTileId = map->getTileIdOf(fighter);

  int distance = map->movementDistance(fromTileId, tileId, fighter);
  if (distance < 0 || distance > remaining) {
    return false;
  }

  map->placeFighter(fighter, tileId);
  movesRemainingByFighter[fighter] = remaining - distance;

  for (auto *obs : observers) obs->onFighterMoved(fighter, fromTileId, tileId);
  return true;
}

bool GameManager::moveThroughFog(Fighter *fighter, int destinationTileId) {
  if (!fighter || matchOver) {
    return false;
  }

  Hero *owner = dynamic_cast<Hero *>(fighter);
  if (!owner) {
    owner = getHero(fighter->getOwnerPlayer());
  }
  if (!isInvisibleMan(owner) || isAbilityDisabled(owner)) {
    return false;
  }

  int fromTileId = map->getTileIdOf(fighter);
  if (fromTileId < 0 || fromTileId == destinationTileId) {
    return false;
  }
  if (!map->hasFogToken(fromTileId) || !map->hasFogToken(destinationTileId)) {
    return false;
  }
  if (map->isOccupied(destinationTileId)) {
    return false;
  }

  int remaining = getMovesRemaining(fighter);
  if (remaining <= 0) {
    return false;
  }

  map->placeFighter(fighter, destinationTileId);
  movesRemainingByFighter[fighter] = remaining - 1;

  for (auto *obs : observers) obs->onFighterMoved(fighter, fromTileId, destinationTileId);
  return true;
}

bool GameManager::canBoostMovement() const {
  return !matchOver && maneuverActive && !maneuverBoosted;
}

void GameManager::finishManeuver() {
  maneuverActive = false;
  maneuverBoosted = false;
  movesRemainingByFighter.clear();
}

bool GameManager::isManeuverActive() const { return maneuverActive; }

bool GameManager::boostMovement(Card *card, Fighter *fighter) {
  if (matchOver || !card || !maneuverActive || maneuverBoosted) {
    return false;
  }

  Hero *hero = getCurrentHero();
  if (!hero || !hero->getDeck()) {
    return false;
  }
  if (!fighter) {
    fighter = hero;
  }

  if (!heroHasCardInHand(hero, card)) {
    return false;
  }

  int gain = card->getBoost();
  hero->getDeck()->discard(card);

  movesRemainingByFighter[fighter] = getMovesRemaining(fighter) + gain;
  maneuverBoosted = true;

  for (auto *obs : observers) obs->onCardDiscarded(hero, card);
  for (auto *obs : observers) obs->onMovementBoosted(hero, card, gain);
  return true;
}

bool GameManager::playCard(Card *card, Fighter *self, Fighter *target, Fighter *enemy) {
  Hero *hero = getCurrentHero();
  if (!hero || !card || !hero->getDeck() || matchOver) {
    return false;
  }
  if (actionsRemaining <= 0) {
    return false;
  }

  if (card->getCardType() != TypeOfCard::event) {
    return false;
  }

  if (!performerAllows(card, self)) {
    return false;
  }

  if (!heroHasCardInHand(hero, card)) {
    return false;
  }

  auto data = std::make_shared<gameData>(
      buildGameData(self, target, enemy, card, nullptr, card->getEventType()));

  int healthBefore = 0;
  if (target) {
    healthBefore = target->getHealth();
  }

  card->resetCancellation();

 bool wasFirstActionThisTurn = (actionsRemaining == 2);
  deferredPlacementRequestedThisCard = false;

  auto &effects = card->getEffects();
  auto index = std::make_shared<size_t>(0);
  auto stepPtr = std::make_shared<std::function<void()>>();
  *stepPtr = [this, &effects, index, data, stepPtr, card, hero, target, healthBefore,
              wasFirstActionThisTurn]() {
    while (*index < effects.size()) {
      Effect *effect = effects[*index].get();
      ++*index;
      if (card->consumeCancellation()) {
        continue;
      }
      effect->execute(*data, [stepPtr]() { (*stepPtr)(); });
      return;
    }

    hero->getDeck()->discard(card);
    actionsRemaining--;

    if (deferredPlacementRequestedThisCard && wasFirstActionThisTurn) {
      actionsRemaining = 0;
    }

    for (auto *obs : observers) obs->onCardPlayed(hero, card);
    if (target) {
      notifyDamage(target, healthBefore);
    }

    checkForGameOver();
  };
  (*stepPtr)();

  return true;
}

bool GameManager::playCard(Card *card) {
  Hero *self = getCurrentHero();
  if (!self) {
    return false;
  }

  Hero *enemy = getNextHero(currentTurn);

  return playCard(card, self, enemy, enemy);
}

Hero *GameManager::getNextHero(int fromPlayer) const {
  for (int i = 1; i < getPlayerCount(); i++) {
    Hero *candidate = getHero((fromPlayer + i) % getPlayerCount());
    if (candidate) {
      return candidate;
    }
  }
  return nullptr;
}

void GameManager::triggerStartOfTurnAbility() {
  Hero *hero = getCurrentHero();
  if (!hero) {
    return;
  }

 resolvePendingStartOfTurnEffects(hero);

  snapshotFogTileAtTurnStart(hero);
  triggerDraculaBloodHarvest(hero);
  checkForGameOver();
}

void GameManager::resolvePendingStartOfTurnEffects(Hero *hero) {
  if (!hero || pendingStartOfTurnEffects.empty()) {
    return;
  }

  std::vector<PendingEffect> due;
  std::vector<PendingEffect> remaining;
  for (auto &entry : pendingStartOfTurnEffects) {
    if (entry.owner && entry.owner->getOwnerPlayer() == hero->getOwnerPlayer()) {
      due.push_back(entry);
    } else {
      remaining.push_back(entry);
    }
  }
  pendingStartOfTurnEffects = std::move(remaining);

  for (auto &entry : due) {
    if (!entry.owner || !entry.effect) {
      continue;
    }
  auto data = std::make_shared<gameData>(
        buildGameData(entry.owner, entry.owner, nullptr, nullptr, nullptr, TypeOfEvent::none));
    entry.effect->executeDeferred(*data, [data]() {});
  }
}

void GameManager::snapshotFogTileAtTurnStart(Hero *hero) {
  if (!hero || !map) {
    return;
  }
 fogTileAtOwnerTurnStart[hero] = map->hasFogToken(map->getTileIdOf(hero));
  for (auto &sk : hero->getSidekicks()) {
    fogTileAtOwnerTurnStart[sk.get()] = map->hasFogToken(map->getTileIdOf(sk.get()));
  }
}

bool GameManager::isAbilityDisabled(const Hero *hero) const {
  return std::find(disabledAbilityHeroes.begin(), disabledAbilityHeroes.end(),
                    hero) != disabledAbilityHeroes.end();
}

void GameManager::triggerDraculaBloodHarvest(Hero *hero) {
  if (!hero || matchOver || !isDracula(hero)) {
    return;
  }
  if (isAbilityDisabled(hero)) {
    return;
  }

  int draculaTile = map->getTileIdOf(hero);

  std::vector<Fighter *> adjacent;
  for (auto &[id, tile] : map->getTiles()) {
    (void)id;
    Fighter *fighter = map->getFighterAt(tile->getId());
    if (!fighter || fighter == hero || !fighter->isAlive()) {
      continue;
    }
    if (map->distanceBetween(draculaTile, tile->getId()) != 1) {
      continue;
    }
    adjacent.push_back(fighter);
  }

  if (adjacent.empty()) {
    return;
  }

  adjacent.push_back(nullptr);

  requestFighterChoice(adjacent, [this, hero](Fighter *chosen) {
    if (!chosen) {
      return;
    }
    chosen->damage(1);
    checkFighterDeath(chosen);
    if (hero->getDeck()) {
      hero->getDeck()->draw(1);
    }
    checkForGameOver();
  });
}

bool GameManager::isAbilityProtected(const Hero *hero) const {
  return isSherlockOrWatson(hero);
}

bool GameManager::disableAbility(Hero *hero) {
  if (!hero || isAbilityProtected(hero)) {
    return false;
  }
  if (!isAbilityDisabled(hero)) {
    disabledAbilityHeroes.push_back(hero);
  }
  return true;
}

void GameManager::enableAbility(Hero *hero) {
  disabledAbilityHeroes.erase(
      std::remove(disabledAbilityHeroes.begin(), disabledAbilityHeroes.end(), hero),
      disabledAbilityHeroes.end());
}

bool GameManager::cardNeedsTarget(const Card *card) const {
  return card && card->needsTarget();
}

std::vector<Fighter *> GameManager::getValidTargetsForCard(const Card *card) const {
  std::vector<Fighter *> targets;
  Hero *self = getCurrentHero();
  if (!card || !self) {
    return targets;
  }

  bool anyFighter = card->isTargetsAnyFighter();
  for (auto &[id, tile] : map->getTiles()) {
    (void)id;
    Fighter *fighter = map->getFighterAt(tile->getId());
    if (fighter && fighter->isAlive() &&
        (anyFighter || fighter->getOwnerPlayer() != self->getOwnerPlayer())) {
      targets.push_back(fighter);
    }
  }
  return targets;
}

gameData GameManager::buildGameData(Fighter *self, Fighter *target, Fighter *enemy,
                                    Card *cardPlayed, Card *enemyCardPlayed,
                                    const TypeOfEvent &event) {
  gameData data;
  data.map = map.get();
  data.self = self;
  data.target = target;
  data.enemy = enemy;
  data.cardPlayed = cardPlayed;
  data.enemyCardPlayed = enemyCardPlayed;
  data.value = 0;
  if (cardPlayed) {
    data.value = cardPlayed->getValue();
  }
  data.event = event;
  data.lastCombatWinner = lastCombatWinner;
  data.lastCombatLoser = lastCombatLoser;
  if (self) {
    auto it = fogTileAtOwnerTurnStart.find(self);
    data.selfStartedTurnOnFogTile = it != fogTileAtOwnerTurnStart.end() && it->second;
  }
  data.requestTileChoice = [this](std::vector<Tile *> options, std::function<void(Tile *)> onChosen) {
    this->requestTileChoice(std::move(options), std::move(onChosen));
  };
  data.requestTileChoiceFor = [this](Fighter *chooser, std::vector<Tile *> options,
                                     std::function<void(Tile *)> onChosen) {
    this->requestTileChoice(chooser, std::move(options), std::move(onChosen));
  };
  data.requestCardChoice = [this](std::vector<Card *> options, std::function<void(Card *)> onChosen) {
    this->requestCardChoice(std::move(options), std::move(onChosen));
  };
  data.requestEffectChoice = [this](std::vector<std::string> labels, std::function<void(int)> onChosen) {
    this->requestEffectChoice(std::move(labels), std::move(onChosen));
  };
  data.requestEffectChoiceFor = [this](Fighter *chooser, std::vector<std::string> labels,
                                       std::function<void(int)> onChosen) {
    this->requestEffectChoice(chooser, std::move(labels), std::move(onChosen));
  };
  data.deferToStartOfNextTurn = [this](Fighter *owner, Effect *effect) {
    if (!owner || !effect) {
      return;
    }
    pendingStartOfTurnEffects.push_back({owner, effect});
    deferredPlacementRequestedThisCard = true;
  };
  data.disableAbility = [this](Hero *hero) { return this->disableAbility(hero); };
  data.grantAction = [this](int amount) { this->actionsRemaining += amount; };
  data.getOwnerHero = [this](Fighter *fighter) -> Hero * {
    if (!fighter) {
      return nullptr;
    }
    if (auto *hero = dynamic_cast<Hero *>(fighter)) {
      return hero;
    }
    return this->getHero(fighter->getOwnerPlayer());
  };
  data.onHandRevealed = [this](Hero *revealedFrom, const std::vector<Card *> &hand) {
    for (auto *obs : observers) obs->onHandSeen(revealedFrom, hand);
  };
  return data;
}

void GameManager::runCombatEvent(const TypeOfEvent &event, Fighter *cardOwner, Card *ownCard,
                                 Fighter *opponent, Card *opponentCard) {
  if (!cardOwner) {
    return;
  }
  if (!ownCard || ownCard->getEventType() != event) {
    return;
  }

auto data = std::make_shared<gameData>(
      buildGameData(cardOwner, opponent, opponent, ownCard, opponentCard, event));

  auto &effects = ownCard->getEffects();
  auto index = std::make_shared<size_t>(0);
  auto stepPtr = std::make_shared<std::function<void()>>();
  *stepPtr = [ownCard, &effects, index, data, stepPtr]() {
    while (*index < effects.size()) {
      Effect *effect = effects[*index].get();
      ++*index;
      if (ownCard->consumeCancellation()) {
        continue;
      }
      effect->execute(*data, [stepPtr]() { (*stepPtr)(); });
      return;
    }
  };
  (*stepPtr)();
}

bool GameManager::startCombat(Card *attackCard, Fighter *attacker, Fighter *target) {
  if (combat.active || matchOver) {
    return false;
  }
  if (actionsRemaining <= 0) {
    return false;
  }

  Hero *hero = getCurrentHero();
  if (!hero || !attacker || !attackCard || !target || !hero->getDeck()) {
    return false;
  }

  if (attacker->getOwnerPlayer() != hero->getOwnerPlayer() || !attacker->isAlive()) {
    return false;
  }
  if (target->getOwnerPlayer() == hero->getOwnerPlayer()) {
    return false;
  }

  if (!heroHasCardInHand(hero, attackCard)) {
    return false;
  }
  if (attackCard->getCardType() != TypeOfCard::attack &&
      attackCard->getCardType() != TypeOfCard::multipurpose) {
    return false;
  }
  if (!performerAllows(attackCard, attacker)) {
    return false;
  }

  int fromTileId = map->getTileIdOf(attacker);
  int targetTileId = map->getTileIdOf(target);
  int distance = map->distanceBetween(fromTileId, targetTileId);

  if (attacker->getTypeOfAttack() == TypeOfAttack::melee) {
    if (distance != 1) {
      return false;
    }
  } else {
    bool adjacent = (distance == 1);
    bool sameZone = false;
    Tile *fromTile = map->getTile(fromTileId);
    Tile *targetTile = map->getTile(targetTileId);
    if (fromTile && targetTile) {
      for (const auto &zone : fromTile->getZones()) {
        if (targetTile->getZones().count(zone)) {
          sameZone = true;
          break;
        }
      }
    }
    if (!adjacent && !sameZone) {
      return false;
    }
  }

  combat = CombatRound{};
  combat.active = true;
  combat.attacker = attacker;
  combat.defender = target;
  combat.attackerCard = attackCard;
  actionsRemaining--;
  return true;
}

bool GameManager::startCombat(Card *attackCard, Fighter *target) {
  return startCombat(attackCard, getCurrentHero(), target);
}

bool GameManager::isCombatActive() const { return combat.active; }
Fighter *GameManager::getCombatAttacker() const { return combat.attacker; }
Fighter *GameManager::getCombatDefender() const { return combat.defender; }
Card *GameManager::getCombatAttackerCard() const { return combat.attackerCard; }

Hero *GameManager::getCombatDefendingHero() const {
  if (!combat.active || !combat.defender) {
    return nullptr;
  }
  if (Hero *asHero = dynamic_cast<Hero *>(combat.defender)) {
    return asHero;
  }

  return getHero(combat.defender->getOwnerPlayer());
}

bool GameManager::resolveCombat(Card *defenseCard, int predictedAttackValue) {
  if (!combat.active) {
    return false;
  }

  Fighter *attacker = combat.attacker;
  Fighter *defender = combat.defender;
  Card *attackerCard = combat.attackerCard;

  Hero *attackingHero = getHero(attacker->getOwnerPlayer());
  Hero *defendingHero = getCombatDefendingHero();

  if (defenseCard) {
    bool ownsCard = heroHasCardInHand(defendingHero, defenseCard);
    bool rightType = defenseCard->getCardType() == TypeOfCard::def ||
                     defenseCard->getCardType() == TypeOfCard::multipurpose;
    bool rightPerformer = performerAllows(defenseCard, defender);
    if (!ownsCard || !rightType || !rightPerformer) {
      defenseCard = nullptr;
    }
  }
  combat.defenderCard = defenseCard;

  attackerCard->setValue(attackerCard->getAttackStat());
  if (defenseCard) {
    defenseCard->setValue(defenseCard->getDefStat());

   int defenderTile = map->getTileIdOf(defender);
    if (defenderTile >= 0 && map->hasFogToken(defenderTile) &&
        isInvisibleMan(defendingHero) && !isAbilityDisabled(defendingHero)) {
      defenseCard->setValue(defenseCard->getValue() + 1);
    }
  }
  attackerCard->resetCancellation();
  if (defenseCard) {
    defenseCard->resetCancellation();
  }

  if (defenseCard) {
    defenseCard->setPredictedValue(predictedAttackValue);
  }

  int defenderHealthBefore = defender->getHealth();

  if (defenseCard) {
    runCombatEvent(TypeOfEvent::before_combat, defender, defenseCard, attacker, attackerCard);
  }
  runCombatEvent(TypeOfEvent::before_combat, attacker, attackerCard, defender, defenseCard);

  if (defenseCard) {
    runCombatEvent(TypeOfEvent::during_combat, defender, defenseCard, attacker, attackerCard);
  }
  runCombatEvent(TypeOfEvent::during_combat, attacker, attackerCard, defender, defenseCard);

  if (isWaitingForSelection()) {
    combatContinuation = [this, attacker, defender, attackerCard, defenseCard, attackingHero,
                           defendingHero, defenderHealthBefore]() {
      finishResolveCombat(attacker, defender, attackerCard, defenseCard, attackingHero,
                           defendingHero, defenderHealthBefore);
    };
    return true;
  }

  finishResolveCombat(attacker, defender, attackerCard, defenseCard, attackingHero, defendingHero,
                       defenderHealthBefore);
  return true;
}

void GameManager::finishResolveCombat(Fighter *attacker, Fighter *defender, Card *attackerCard,
                                       Card *defenseCard, Hero *attackingHero, Hero *defendingHero,
                                       int defenderHealthBefore) {
  int attackValue = attackerCard->getValue();
  int defValue = 0;
  if (defenseCard) {
    defValue = defenseCard->getValue();
  }

  Fighter *winner{nullptr};
  Fighter *loser{nullptr};
  int damage = 0;
  if (attackValue > defValue) {
    winner = attacker;
    loser = defender;
    damage = attackValue - defValue;
    defender->damage(damage);
  } else {

    winner = defender;
    loser = attacker;
  }
  lastCombatWinner = winner;
  lastCombatLoser = loser;

  if (winner == attacker) {
    runCombatEvent(TypeOfEvent::after_combat, attacker, attackerCard, defender, defenseCard);
    if (defenseCard) {
      runCombatEvent(TypeOfEvent::after_combat, defender, defenseCard, attacker, attackerCard);
    }
  } else {
    if (defenseCard) {
      runCombatEvent(TypeOfEvent::after_combat, defender, defenseCard, attacker, attackerCard);
    }
    runCombatEvent(TypeOfEvent::after_combat, attacker, attackerCard, defender, defenseCard);
  }

  if (attackingHero && attackingHero->getDeck()) {
    attackingHero->getDeck()->discard(attackerCard);
  }
  if (defenseCard && defendingHero && defendingHero->getDeck()) {
    defendingHero->getDeck()->discard(defenseCard);
  }

  if (attackingHero) {
    for (auto *obs : observers) obs->onCardPlayed(attackingHero, attackerCard);
  }
  if (defenseCard && defendingHero) {
    for (auto *obs : observers) obs->onCardPlayed(defendingHero, defenseCard);
  }
  for (auto *obs : observers) obs->onCombatResolved(attacker, defender, winner, loser, damage);
  notifyDamage(defender, defenderHealthBefore);

  combat = CombatRound{};
  checkForGameOver();
}

void GameManager::cancelCombat() {
  combat = CombatRound{};
  combatContinuation = nullptr;
}

void GameManager::runCombatContinuationIfReady() {
  if (!combatContinuation || isWaitingForSelection()) {
    return;
  }
  auto cont = std::move(combatContinuation);
  combatContinuation = nullptr;
  cont();
}

void GameManager::requestTileChoice(std::vector<Tile *> options, std::function<void(Tile *)> onChosen) {
  requestTileChoice(nullptr, std::move(options), std::move(onChosen));
}
void GameManager::requestTileChoice(Fighter *chooser, std::vector<Tile *> options, std::function<void(Tile *)> onChosen) {
  pendingChooser = chooser;
  pending = std::move(options);
  onSelectionResolved = [onChosen](void *raw) { onChosen(static_cast<Tile *>(raw)); };
}
void GameManager::requestFighterChoice(std::vector<Fighter *> options, std::function<void(Fighter *)> onChosen) {
  pendingChooser = nullptr;
  pending = std::move(options);
  onSelectionResolved = [onChosen](void *raw) { onChosen(static_cast<Fighter *>(raw)); };
}
void GameManager::requestCardChoice(std::vector<Card *> options, std::function<void(Card *)> onChosen) {
  pendingChooser = nullptr;
  pending = std::move(options);
  onSelectionResolved = [onChosen](void *raw) { onChosen(static_cast<Card *>(raw)); };
}

void GameManager::requestEffectChoice(std::vector<std::string> labels, std::function<void(int)> onChosen) {
  requestEffectChoice(nullptr, std::move(labels), std::move(onChosen));
}
void GameManager::requestEffectChoice(Fighter *chooser, std::vector<std::string> labels,
                                      std::function<void(int)> onChosen) {
  pendingChooser = chooser;
  pendingEffectChoiceLabels = std::move(labels);
  waitingForEffectChoice = true;
  onEffectChoiceResolved = std::move(onChosen);
}

bool GameManager::isWaitingForTile() const {
  return std::holds_alternative<std::vector<Tile *>>(pending);
}
bool GameManager::isWaitingForFighter() const {
  return std::holds_alternative<std::vector<Fighter *>>(pending);
}
bool GameManager::isWaitingForCard() const {
  return std::holds_alternative<std::vector<Card *>>(pending);
}
bool GameManager::isWaitingForEffectChoice() const {
  return waitingForEffectChoice;
}
bool GameManager::isWaitingForSelection() const {
  return !std::holds_alternative<std::monostate>(pending) || waitingForEffectChoice;
}

std::vector<Tile *> GameManager::getValidTiles() const {
  return std::get<std::vector<Tile *>>(pending);
}

Tile *GameManager::getStayTileOption() const {
  if (!isWaitingForTile() || !map) {
    return nullptr;
  }
  for (Tile *t : getValidTiles()) {
    if (t && map->isOccupied(t->getId())) {
      return t;
    }
  }
  return nullptr;
}
std::vector<Fighter *> GameManager::getValidFighters() const {
  return std::get<std::vector<Fighter *>>(pending);
}
std::vector<Card *> GameManager::getValidCards() const {
  return std::get<std::vector<Card *>>(pending);
}
std::vector<std::string> GameManager::getValidEffectChoiceLabels() const {
  return pendingEffectChoiceLabels;
}

void GameManager::submitTile(Tile *chosen) {
  if (!isWaitingForTile()) {
    return;
  }
  auto &options = std::get<std::vector<Tile *>>(pending);
  if (std::find(options.begin(), options.end(), chosen) == options.end()) {
    return;
  }

  auto callback = onSelectionResolved;
  clearPending();
  if (callback) {
    callback(chosen);
  }
  runCombatContinuationIfReady();
}
void GameManager::submitFighter(Fighter *chosen) {
  if (!isWaitingForFighter()) {
    return;
  }
  auto &options = std::get<std::vector<Fighter *>>(pending);
  if (std::find(options.begin(), options.end(), chosen) == options.end()) {
    return;
  }

  auto callback = onSelectionResolved;
  clearPending();
  if (callback) {
    callback(chosen);
  }
  runCombatContinuationIfReady();
}
void GameManager::submitCard(Card *chosen) {
  if (!isWaitingForCard()) {
    return;
  }
  auto &options = std::get<std::vector<Card *>>(pending);
  if (std::find(options.begin(), options.end(), chosen) == options.end()) {
    return;
  }

  auto callback = onSelectionResolved;
  clearPending();
  if (callback) {
    callback(chosen);
  }
  runCombatContinuationIfReady();
}
void GameManager::submitEffectChoice(int index) {
  if (!waitingForEffectChoice) {
    return;
  }
  if (index < 0 || static_cast<std::size_t>(index) >= pendingEffectChoiceLabels.size()) {
    return;
  }

  auto callback = onEffectChoiceResolved;
  waitingForEffectChoice = false;
  pendingEffectChoiceLabels.clear();
  onEffectChoiceResolved = nullptr;
  pendingChooser = nullptr;
  if (callback) {
    callback(index);
  }
  runCombatContinuationIfReady();
}
void GameManager::clearPending() {
  pending = std::monostate{};
  onSelectionResolved = nullptr;
  pendingChooser = nullptr;
  waitingForEffectChoice = false;
  pendingEffectChoiceLabels.clear();
  onEffectChoiceResolved = nullptr;
}