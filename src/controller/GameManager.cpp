#include "controller/GameManager.h"
#include "controller/PlayerSelectionManager.h"
#include "factory/heroFactory.h"
#include "factory/mapFactory.h"
#include "model/sidekick.h"
#include "model/deck.h"
#include "model/typeOfTile.h"
#include "engine/effects/effect.h"
#include "engine/effects/choosePlaceEffect.h"
#include "libraries/magic_enum.hpp"
#include "utility/exceptions.h"
#include "utility/file.h"
#include <algorithm>
#include <filesystem>
#include <fstream>

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

  std::string idForFighter(const std::vector<std::unique_ptr<Hero>> &heroes,
                           const Fighter *fighter) {
    for (auto &hero : heroes) {
      if (hero.get() == fighter) {
        return hero->getName();
      }
      auto &sidekicks = hero->getSidekicks();
      for (std::size_t i = 0; i < sidekicks.size(); i++) {
        if (sidekicks[i].get() == fighter) {
          return hero->getName() + "_" + std::to_string(i);
        }
      }
    }
    return "";
  }

  Fighter *fighterForId(const std::vector<std::unique_ptr<Hero>> &heroes,
                        const std::string &id) {
    for (auto &hero : heroes) {
      if (hero->getName() == id) {
        return hero.get();
      }
    }
    auto sep = id.find_last_of('_');
    if (sep == std::string::npos) {
      return nullptr;
    }
    std::string heroName = id.substr(0, sep);
    try {
      int idx = std::stoi(id.substr(sep + 1));
      for (auto &hero : heroes) {
        if (hero->getName() != heroName) {
          continue;
        }
        auto &sidekicks = hero->getSidekicks();
        if (idx >= 0 && (std::size_t)idx < sidekicks.size()) {
          return sidekicks[idx].get();
        }
      }
    } catch (const std::exception &) {
    }
    return nullptr;
  }

  Hero *heroForName(const std::vector<std::unique_ptr<Hero>> &heroes,
                    const std::string &name) {
    for (auto &hero : heroes) {
      if (hero->getName() == name) {
        return hero.get();
      }
    }
    return nullptr;
  }

  std::unique_ptr<Card> extractCardByName(std::vector<std::unique_ptr<Card>> &pool,
                                          const std::string &name) {
    for (auto it = pool.begin(); it != pool.end(); ++it) {
      if (*it && (*it)->getName() == name) {
        auto card = std::move(*it);
        pool.erase(it);
        return card;
      }
    }
    return nullptr;
  }

  nlohmann::json serializeDeck(Deck *deck) {
    nlohmann::json d = nlohmann::json::object();

    nlohmann::json hand = nlohmann::json::array();
    for (Card *c : deck->getHand()) {
      hand.push_back(c->getName());
    }
    nlohmann::json drawPile = nlohmann::json::array();
    for (Card *c : deck->getDrawPile()) {
      drawPile.push_back(c->getName());
    }
    nlohmann::json discardPile = nlohmann::json::array();
    for (Card *c : deck->getDiscardPile()) {
      discardPile.push_back(c->getName());
    }

    d["hand"] = std::move(hand);
    d["drawPile"] = std::move(drawPile);
    d["discardPile"] = std::move(discardPile);
    return d;
  }

  std::vector<std::unique_ptr<Card>> extractCardsByName(std::vector<std::unique_ptr<Card>> &pool,
                                                        const nlohmann::json &names) {
    std::vector<std::unique_ptr<Card>> result;
    if (!names.is_array()) {
      return result;
    }
    for (auto &nameJson : names) {
      if (!nameJson.is_string()) {
        continue;
      }
      if (auto card = extractCardByName(pool, nameJson.get<std::string>())) {
        result.push_back(std::move(card));
      }
    }
    return result;
  }
}

GameManager::GameManager(std::vector<std::unique_ptr<Hero>> heroesIn, std::unique_ptr<Map> mapIn)
    : heroes(std::move(heroesIn)), map(std::move(mapIn)) {
  for (int i = 0; i < (int)heroes.size(); i++) {
    heroes[i]->setOwnerPlayer(i);
    for (auto &sidekick : heroes[i]->getSidekicks()) {
      sidekick->setOwnerPlayer(i);
    }
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
    auto gm = std::make_unique<GameManager>(std::move(heroes), std::move(map));
    gm->vsAI = PlayerSelectionManager::instance().isVsAI();
    return gm;
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

nlohmann::json GameManager::serializeState() const {
  nlohmann::json j;
  j["mapName"] = map->getName();
  j["vsAI"] = vsAI;

  nlohmann::json fogArr = nlohmann::json::array();
  for (int tileId : map->getFogTokenTileIds()) {
    fogArr.push_back({{"tileId", tileId}, {"count", map->fogTokenCountAt(tileId)}});
  }
  j["fogTokens"] = std::move(fogArr);

  nlohmann::json heroesArr = nlohmann::json::array();
  for (auto &hero : heroes) {
    nlohmann::json h;
    h["name"] = hero->getName();
    h["tileId"] = map->getTileIdOf(hero.get());
    h["health"] = hero->getHealth();

    h["fatigued"] = hero->isFatigued();
    h["startingFogTokenCount"] = hero->getStartingFogTokenCount();
    if (hero->getDeck()) {
      h["deck"] = serializeDeck(hero->getDeck());
    }

    nlohmann::json sidekicksArr = nlohmann::json::array();
    for (auto &sk : hero->getSidekicks()) {
      nlohmann::json s;
      s["name"] = sk->getName();
      s["tileId"] = map->getTileIdOf(sk.get());
      s["health"] = sk->getHealth();
      s["fatigued"] = sk->isFatigued();
      sidekicksArr.push_back(std::move(s));
    }
    h["sidekicks"] = std::move(sidekicksArr);

    heroesArr.push_back(std::move(h));
  }
  j["heroes"] = std::move(heroesArr);

  j["currentTurn"] = currentTurn;
  j["actionsRemaining"] = actionsRemaining;
  j["maneuverActive"] = maneuverActive;
  j["maneuverBoosted"] = maneuverBoosted;

  nlohmann::json movesObj = nlohmann::json::object();
  for (auto &entry : movesRemainingByFighter) {
    std::string id = idForFighter(heroes, entry.first);
    if (!id.empty()) {
      movesObj[id] = entry.second;
    }
  }
  j["movesRemainingByFighter"] = std::move(movesObj);

  nlohmann::json fogTurnObj = nlohmann::json::object();
  for (auto &entry : fogTileAtOwnerTurnStart) {
    std::string id = idForFighter(heroes, entry.first);
    if (!id.empty()) {
      fogTurnObj[id] = entry.second;
    }
  }
  j["fogTileAtOwnerTurnStart"] = std::move(fogTurnObj);

  nlohmann::json disabledArr = nlohmann::json::array();
  for (Hero *h : disabledAbilityHeroes) {
    if (h) {
      disabledArr.push_back(h->getName());
    }
  }
  j["disabledAbilityHeroes"] = std::move(disabledArr);


  nlohmann::json pendingArr = nlohmann::json::array();
  for (auto &entry : pendingStartOfTurnEffects) {
    std::string id = idForFighter(heroes, entry.owner);
    if (!id.empty()) {
      pendingArr.push_back({{"owner", id}, {"kind", "choose_place_self"}});
    }
  }
  j["pendingStartOfTurnEffects"] = std::move(pendingArr);

  return j;
}

bool GameManager::saveGame(int slot) const {
  if (isWaitingForSelection() || isCombatActive()) {
    return false;
  }
  if (!map) {
    return false;
  }

  try {
    nlohmann::json j = serializeState();

    std::filesystem::path path(saveFilePath(slot));
    if (path.has_parent_path()) {
      std::filesystem::create_directories(path.parent_path());
    }
    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open()) {
      return false;
    }
    out << j.dump(2);
    return out.good();
  } catch (const std::exception &) {
    return false;
  }
}

std::unique_ptr<GameManager> GameManager::loadGame(int slot) {
  if (!hasSave(slot)) {
    return nullptr;
  }

  try {
    nlohmann::json j = load(saveFilePath(slot));
    return buildFromState(j);
  } catch (const AppException &) {
    return nullptr;
  } catch (const nlohmann::json::exception &) {
    return nullptr;
  } catch (const std::exception &) {
    return nullptr;
  }
}

std::unique_ptr<GameManager> GameManager::buildFromState(const nlohmann::json &j) {
  try {
    if (!j.contains("heroes") || !j["heroes"].is_array() || !j.contains("mapName")) {
      return nullptr;
    }

    std::vector<std::unique_ptr<Hero>> heroes;
    for (auto &hJson : j["heroes"]) {
      std::string name = hJson.value("name", "");
      auto hero = HeroFactory::create(name);

      hero->setHealth(hJson.value("health", hero->getHealth()));
      hero->setFatigued(hJson.value("fatigued", false));
      hero->setStartingFogTokenCount(
          hJson.value("startingFogTokenCount", hero->getStartingFogTokenCount()));

      if (hero->getDeck() && hJson.contains("deck") && hJson["deck"].is_object()) {
        const nlohmann::json &deckJson = hJson["deck"];
        auto pool = hero->getDeck()->releaseAllCards();

        auto newHand = extractCardsByName(pool, deckJson.value("hand", nlohmann::json::array()));
        auto newDraw =
            extractCardsByName(pool, deckJson.value("drawPile", nlohmann::json::array()));
        auto newDiscard =
            extractCardsByName(pool, deckJson.value("discardPile", nlohmann::json::array()));

        hero->getDeck()->restoreState(std::move(newHand), std::move(newDraw),
                                      std::move(newDiscard));
      }

      if (hJson.contains("sidekicks") && hJson["sidekicks"].is_array()) {
        auto &sidekickJsonArr = hJson["sidekicks"];
        auto &sidekicks = hero->getSidekicks();
        for (std::size_t i = 0; i < sidekicks.size() && i < sidekickJsonArr.size(); i++) {
          const nlohmann::json &skJson = sidekickJsonArr[i];
          sidekicks[i]->setHealth(skJson.value("health", sidekicks[i]->getHealth()));
          sidekicks[i]->setFatigued(skJson.value("fatigued", false));
        }
      }

      heroes.push_back(std::move(hero));
    }

    auto map = MapFactory::create(j.value("mapName", ""));

    for (std::size_t hi = 0; hi < heroes.size(); hi++) {
      const nlohmann::json &hJson = j["heroes"][hi];
      Hero *hero = heroes[hi].get();

      int heroTile = hJson.value("tileId", -1);
      if (hero->isAlive() && heroTile >= 0) {
        map->placeFighter(hero, heroTile);
      }

      if (hJson.contains("sidekicks") && hJson["sidekicks"].is_array()) {
        const nlohmann::json &sidekickJsonArr = hJson["sidekicks"];
        auto &sidekicks = hero->getSidekicks();
        for (std::size_t i = 0; i < sidekicks.size() && i < sidekickJsonArr.size(); i++) {
          int skTile = sidekickJsonArr[i].value("tileId", -1);
          if (sidekicks[i]->isAlive() && skTile >= 0) {
            map->placeFighter(sidekicks[i].get(), skTile);
          }
        }
      }
    }

    if (j.contains("fogTokens") && j["fogTokens"].is_array()) {
      for (auto &fogJson : j["fogTokens"]) {
        int tileId = fogJson.value("tileId", -1);
        int count = fogJson.value("count", 0);
        // Only one mist token can ever occupy a tile, regardless of what a
        // (possibly stale) save file's "count" field says.
        if (count > 0) {
          map->addFogToken(tileId);
        }
      }
    }

    auto gm = std::make_unique<GameManager>(std::move(heroes), std::move(map));

    gm->vsAI = j.value("vsAI", false);
    gm->currentTurn = j.value("currentTurn", 0);
    gm->actionsRemaining = j.value("actionsRemaining", 0);
    gm->maneuverActive = j.value("maneuverActive", false);
    gm->maneuverBoosted = j.value("maneuverBoosted", false);

    if (j.contains("movesRemainingByFighter") && j["movesRemainingByFighter"].is_object()) {
      for (auto &[id, amount] : j["movesRemainingByFighter"].items()) {
        if (Fighter *f = fighterForId(gm->heroes, id)) {
          gm->movesRemainingByFighter[f] = amount.get<int>();
        }
      }
    }

    if (j.contains("fogTileAtOwnerTurnStart") && j["fogTileAtOwnerTurnStart"].is_object()) {
      for (auto &[id, onFog] : j["fogTileAtOwnerTurnStart"].items()) {
        if (Fighter *f = fighterForId(gm->heroes, id)) {
          gm->fogTileAtOwnerTurnStart[f] = onFog.get<bool>();
        }
      }
    }

    if (j.contains("disabledAbilityHeroes") && j["disabledAbilityHeroes"].is_array()) {
      for (auto &nameJson : j["disabledAbilityHeroes"]) {
        if (!nameJson.is_string()) {
          continue;
        }
        if (Hero *h = heroForName(gm->heroes, nameJson.get<std::string>())) {
          gm->disabledAbilityHeroes.push_back(h);
        }
      }
    }

    if (j.contains("pendingStartOfTurnEffects") && j["pendingStartOfTurnEffects"].is_array()) {
      for (auto &entryJson : j["pendingStartOfTurnEffects"]) {
        std::string ownerId = entryJson.value("owner", "");
        Fighter *owner = fighterForId(gm->heroes, ownerId);
        if (!owner) {
          continue;
        }
        auto effect = std::make_unique<ChoosePlaceEffect>("self", "start_of_next_turn");
        Effect *effectPtr = effect.get();
        gm->ownedDeferredEffects.push_back(std::move(effect));
        gm->pendingStartOfTurnEffects.push_back({owner, effectPtr});
      }
    }

    gm->checkForGameOver();

    return gm;
  } catch (const AppException &) {
    return nullptr;
  } catch (const nlohmann::json::exception &) {
    return nullptr;
  } catch (const std::exception &) {
    return nullptr;
  }
}

void GameManager::pushUndoCheckpoint() {
  if (isWaitingForSelection() || isCombatActive() || !map) {
    return;
  }
  try {
    undoStack.push_back(serializeState());
  } catch (const std::exception &) {
  }
}

void GameManager::clearUndoHistory() { undoStack.clear(); }

bool GameManager::canUndo() const { return !undoStack.empty(); }

std::unique_ptr<GameManager> GameManager::undo() {
  if (undoStack.empty()) {
    return nullptr;
  }

  nlohmann::json snapshot = std::move(undoStack.back());
  undoStack.pop_back();

  auto restored = buildFromState(snapshot);
  if (!restored) {
    return nullptr;
  }

  restored->undoStack = std::move(undoStack);
  return restored;
}

void GameManager::resetTurnState() {
  clearUndoHistory();
  actionsRemaining = 2;
  movesRemainingByFighter.clear();
  maneuverActive = false;
  maneuverBoosted = false;
  clearPending();
  cancelCombat();

  if (Hero *hero = getCurrentHero()) {
    hero->setFatigued(false);
    for (auto &sk : hero->getSidekicks())
      sk->setFatigued(false);
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

    std::vector<int> remaining;
    for (int id : availableStartTileIds) {
      if (id != chosen->getId()) {
        remaining.push_back(id);
      }
    }
    placeHeroesFrom(heroIndex + 1, remaining); });
}

void GameManager::placeSidekicksFrom(int heroIndex, std::size_t sidekickIndex) {

  while (heroIndex < (int)heroes.size() &&
         sidekickIndex >= heroes[heroIndex]->getSidekicks().size()) {
    heroIndex++;
    sidekickIndex = 0;
  }

  if (heroIndex >= (int)heroes.size()) {

    sidekickAwaitingPlacement = nullptr;
    placeFogTokensFrom(0, 0);
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
              [](Tile *a, Tile *b)
              { return a->getId() < b->getId(); });
  }

  if (options.empty()) {

    placeSidekicksFrom(heroIndex, sidekickIndex + 1);
    return;
  }

  requestTileChoice(options, [this, heroIndex, sidekickIndex, sidekick](Tile *chosen) {
    map->placeFighter(sidekick, chosen->getId());
    placeSidekicksFrom(heroIndex, sidekickIndex + 1); });
}

void GameManager::placeFogTokensFrom(int heroIndex, int fogIndex) {

  while (heroIndex < (int)heroes.size() &&
         fogIndex >= heroes[heroIndex]->getStartingFogTokenCount()) {
    heroIndex++;
    fogIndex = 0;
  }

  if (heroIndex >= (int)heroes.size()) {

    fogTokenAwaitingHero = nullptr;
    finishSetup();
    return;
  }

  Hero *hero = heroes[heroIndex].get();
  fogTokenAwaitingHero = hero;

  int heroTileId = map->getTileIdOf(hero);
  Tile *heroTile = map->getTile(heroTileId);
  std::vector<Tile *> options;
  if (heroTile) {
    for (Tile *t : map->getTilesInZones(heroTile->getZones())) {
      if (t->getId() == heroTileId || map->isOccupied(t->getId()) ||
          map->hasFogToken(t->getId())) {
        // Skip the hero's own tile, tiles occupied by a fighter, and any
        // tile that already holds a mist token (only one mist token per
        // tile is allowed at a time).
        continue;
      }
      options.push_back(t);
    }
    std::sort(options.begin(), options.end(),
              [](Tile *a, Tile *b)
              { return a->getId() < b->getId(); });
  }

  if (options.empty()) {

    placeFogTokensFrom(heroIndex, fogIndex + 1);
    return;
  }

  requestTileChoice(options, [this, heroIndex, fogIndex](Tile *chosen) {
    map->addFogToken(chosen->getId());
    placeFogTokensFrom(heroIndex, fogIndex + 1); });
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

  for (auto *obs : observers)
    obs->onGameStarted();
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

  if (aliveCount <= 1 && heroes.size() > 1) {
    matchOver = true;
    winningHero = lastStanding;
    for (auto *obs : observers)
      obs->onGameOver(winningHero);
  }
}

bool GameManager::endTurn() {
  if (heroes.empty() || matchOver) {
    return false;
  }
  if (isWaitingForSelection() || isCombatActive()) {
    return false;
  }
  if (actionsRemaining > 0) {
    return false;
  }
  if (needsEndOfTurnDiscard()) {
    return false;
  }

  int playerCount = (int)heroes.size();
  for (int i = 0; i < playerCount; i++) {
    currentTurn = (currentTurn + 1) % playerCount;
    Hero *next = heroes[currentTurn].get();
    if (next && next->isAlive()) {
      break;
    }
  }
  resetTurnState();
  triggerStartOfTurnAbility();

  for (auto *obs : observers)
    obs->onTurnEnded(currentTurn);
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
  for (auto *obs : observers) {
    obs->onFighterRemoved(fighter);
  }
}

void GameManager::reapTheDead() {
  for (auto &hero : heroes) {
    checkFighterDeath(hero.get());
    for (auto &sk : hero->getSidekicks()) {
      checkFighterDeath(sk.get());
    }
  }
}

void GameManager::notifyDamage(Fighter *fighter, int healthBefore) {
  if (!fighter) {
    return;
  }
  int lost = healthBefore - fighter->getHealth();
  if (lost > 0) {
    for (auto *obs : observers)
      obs->onFighterDamaged(fighter, lost);
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
  for (auto &sk : hero->getSidekicks())
    hit(sk.get());
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

  pushUndoCheckpoint();

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

  for (auto *obs : observers)
    obs->onManeuverPerformed(hero, deckWasEmpty);
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

  pushUndoCheckpoint();

  hero->getDeck()->discard(card);
  for (auto *obs : observers)
    obs->onCardDiscarded(hero, card);
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

  pushUndoCheckpoint();

  map->placeFighter(fighter, tileId);
  movesRemainingByFighter[fighter] = remaining - distance;

  for (auto *obs : observers)
    obs->onFighterMoved(fighter, fromTileId, tileId);
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

  pushUndoCheckpoint();

  map->placeFighter(fighter, destinationTileId);
  movesRemainingByFighter[fighter] = remaining - 1;

  for (auto *obs : observers)
    obs->onFighterMoved(fighter, fromTileId, destinationTileId);
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

  pushUndoCheckpoint();

  int gain = card->getBoost();
  hero->getDeck()->discard(card);

  movesRemainingByFighter[fighter] = getMovesRemaining(fighter) + gain;
  maneuverBoosted = true;

  for (auto *obs : observers)
    obs->onCardDiscarded(hero, card);
  for (auto *obs : observers)
    obs->onMovementBoosted(hero, card, gain);
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

  pushUndoCheckpoint();

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
      effect->execute(*data, [stepPtr]()
                      { (*stepPtr)(); });
      return;
    }

    hero->getDeck()->discard(card);
    actionsRemaining--;

    if (deferredPlacementRequestedThisCard && wasFirstActionThisTurn) {
      actionsRemaining = 0;
    }

    for (auto *obs : observers)
      obs->onCardPlayed(hero, card);
    if (target) {
      notifyDamage(target, healthBefore);
    }
    reapTheDead();

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
    checkForGameOver(); });
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
  bool adjacentOnly = card->needsAdjacentTarget();
  int selfTile = adjacentOnly ? map->getTileIdOf(self) : -1;

  for (auto &[id, tile] : map->getTiles()) {
    (void)id;
    Fighter *fighter = map->getFighterAt(tile->getId());
    if (!fighter || !fighter->isAlive()) {
      continue;
    }
    if (!(anyFighter || fighter->getOwnerPlayer() != self->getOwnerPlayer())) {
      continue;
    }
    if (adjacentOnly) {
      int d = map->distanceBetween(selfTile, tile->getId());
      if (d < 0 || d > 1) {
        continue;
      }
    }
    targets.push_back(fighter);
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
  data.requestTileChoiceWithStay = [this](std::vector<Tile *> options, Tile *explicitStayTile,
                                          std::function<void(Tile *)> onChosen) {
    this->requestTileChoice(std::move(options), std::move(onChosen), explicitStayTile);
  };
  data.requestCardChoice = [this](std::vector<Card *> options, std::function<void(Card *)> onChosen) {
    this->requestCardChoice(std::move(options), std::move(onChosen));
  };
  data.requestCardChoiceFor = [this](Fighter *chooser, std::vector<Card *> options,
                                     std::function<void(Card *)> onChosen) {
    this->requestCardChoice(chooser, std::move(options), std::move(onChosen));
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
  data.disableAbility = [this](Hero *hero)
  { return this->disableAbility(hero); };
  data.grantAction = [this](int amount)
  { this->actionsRemaining += amount; };
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
    for (auto *obs : observers)
      obs->onHandSeen(revealedFrom, hand);
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
      effect->execute(*data, [stepPtr]()
                      { (*stepPtr)(); });
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
  if (!target->isAlive() || target->getOwnerPlayer() == hero->getOwnerPlayer()) {
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

  pushUndoCheckpoint();

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

  attackerCard->unlockValue();
  if (defenseCard) {
    defenseCard->unlockValue();
  }

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

  runCombatEvent(TypeOfEvent::before_combat, attacker, attackerCard, defender, defenseCard);
  if (defenseCard) {
    runCombatEvent(TypeOfEvent::before_combat, defender, defenseCard, attacker, attackerCard);
  }

  runCombatEvent(TypeOfEvent::during_combat, attacker, attackerCard, defender, defenseCard);
  if (defenseCard) {
    runCombatEvent(TypeOfEvent::during_combat, defender, defenseCard, attacker, attackerCard);
  }

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
  } else if (defenseCard) {
    runCombatEvent(TypeOfEvent::after_combat, defender, defenseCard, attacker, attackerCard);
  }

  if (isWaitingForSelection()) {
    combatContinuation = [this, attacker, defender, attackerCard, defenseCard, attackingHero,
                          defendingHero, winner, loser, damage, defenderHealthBefore]() {
      runSecondAfterCombatEvent(attacker, defender, attackerCard, defenseCard, attackingHero,
                                defendingHero, winner, loser, damage, defenderHealthBefore);
    };
    return;
  }

  runSecondAfterCombatEvent(attacker, defender, attackerCard, defenseCard, attackingHero,
                            defendingHero, winner, loser, damage, defenderHealthBefore);
}

void GameManager::runSecondAfterCombatEvent(Fighter *attacker, Fighter *defender,
                                            Card *attackerCard, Card *defenseCard,
                                            Hero *attackingHero, Hero *defendingHero,
                                            Fighter *winner, Fighter *loser, int damage,
                                            int defenderHealthBefore) {
  if (winner == attacker) {
    if (defenseCard) {
      runCombatEvent(TypeOfEvent::after_combat, defender, defenseCard, attacker, attackerCard);
    }
  } else {
    runCombatEvent(TypeOfEvent::after_combat, attacker, attackerCard, defender, defenseCard);
  }

  if (isWaitingForSelection()) {
    combatContinuation = [this, attacker, defender, attackerCard, defenseCard, attackingHero,
                          defendingHero, winner, loser, damage, defenderHealthBefore]() {
      finalizeCombat(attacker, defender, attackerCard, defenseCard, attackingHero, defendingHero,
                     winner, loser, damage, defenderHealthBefore);
    };
    return;
  }

  finalizeCombat(attacker, defender, attackerCard, defenseCard, attackingHero, defendingHero,
                winner, loser, damage, defenderHealthBefore);
}

void GameManager::finalizeCombat(Fighter *attacker, Fighter *defender, Card *attackerCard,
                                 Card *defenseCard, Hero *attackingHero, Hero *defendingHero,
                                 Fighter *winner, Fighter *loser, int damage,
                                 int defenderHealthBefore) {
  if (attackingHero && attackingHero->getDeck()) {
    attackingHero->getDeck()->discard(attackerCard);
  }
  if (defenseCard && defendingHero && defendingHero->getDeck()) {
    defendingHero->getDeck()->discard(defenseCard);
  }

  if (attackingHero) {
    for (auto *obs : observers)
      obs->onCardPlayed(attackingHero, attackerCard);
  }
  if (defenseCard && defendingHero) {
    for (auto *obs : observers)
      obs->onCardPlayed(defendingHero, defenseCard);
  }
  for (auto *obs : observers)
    obs->onCombatResolved(attacker, defender, winner, loser, damage);
  notifyDamage(defender, defenderHealthBefore);
  reapTheDead();

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

void GameManager::requestTileChoice(std::vector<Tile *> options, std::function<void(Tile *)> onChosen,
                                    Tile *explicitStayTile) {
  requestTileChoice(nullptr, std::move(options), std::move(onChosen), explicitStayTile);
}
void GameManager::requestTileChoice(Fighter *chooser, std::vector<Tile *> options,
                                    std::function<void(Tile *)> onChosen, Tile *explicitStayTile) {
  pendingChooser = chooser;

  pendingExplicitStayTile = explicitStayTile;
  pending = std::move(options);
  onSelectionResolved = [onChosen](void *raw)
  { onChosen(static_cast<Tile *>(raw)); };
}
void GameManager::requestFighterChoice(std::vector<Fighter *> options, std::function<void(Fighter *)> onChosen) {
  pendingChooser = nullptr;
  pendingExplicitStayTile = nullptr;
  pending = std::move(options);
  onSelectionResolved = [onChosen](void *raw)
  { onChosen(static_cast<Fighter *>(raw)); };
}
void GameManager::requestCardChoice(std::vector<Card *> options, std::function<void(Card *)> onChosen) {
  requestCardChoice(nullptr, std::move(options), std::move(onChosen));
}

void GameManager::requestCardChoice(Fighter *chooser, std::vector<Card *> options, std::function<void(Card *)> onChosen) {
  pendingChooser = chooser;
  pendingExplicitStayTile = nullptr;
  pending = std::move(options);
  onSelectionResolved = [onChosen](void *raw)
  { onChosen(static_cast<Card *>(raw)); };
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
  if (!isWaitingForTile() || !map || !pendingExplicitStayTile) {
    return nullptr;
  }
  for (Tile *t : getValidTiles()) {
    if (t == pendingExplicitStayTile) {
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