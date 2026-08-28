#pragma once
#include "model/card.h"
#include "model/map.h"
#include "model/hero.h"
#include "engine/gameData.h"
#include "engine/observer/observer.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <nlohmann/json.hpp>

using PendingSelection =
    std::variant<std::monostate, std::vector<Tile *>, std::vector<Fighter *>,
                 std::vector<Card *>>;

class Effect;

class GameManager {
private:
  std::vector<std::unique_ptr<Hero>> heroes;
  std::unique_ptr<Map> map;
  int currentTurn{0};

  bool vsAI{false};

  std::unordered_map<Fighter *, int> movesRemainingByFighter;
  int actionsRemaining{0};
  bool maneuverActive{false};
  bool maneuverBoosted{false};

  std::vector<IGameObserver *> observers;

  PendingSelection pending;
  std::function<void(void *)> onSelectionResolved;

  Fighter *pendingChooser{nullptr};
  Tile *pendingExplicitStayTile{nullptr};

  std::vector<std::string> pendingEffectChoiceLabels;
  bool waitingForEffectChoice{false};
  std::function<void(int)> onEffectChoiceResolved;

  struct PendingEffect {
    Fighter *owner{nullptr};
    Effect *effect{nullptr};
  };
  std::vector<PendingEffect> pendingStartOfTurnEffects;
  void resolvePendingStartOfTurnEffects(Hero *hero);
  std::vector<std::unique_ptr<Effect>> ownedDeferredEffects;

  bool deferredPlacementRequestedThisCard{false};

  struct CombatRound {
    bool active{false};
    Fighter *attacker{nullptr};
    Fighter *defender{nullptr};
    Card *attackerCard{nullptr};
    Card *defenderCard{nullptr};
  };
  CombatRound combat;

  std::function<void()> combatContinuation;
  void runCombatContinuationIfReady();
  void finishResolveCombat(Fighter *attacker, Fighter *defender, Card *attackerCard,
                            Card *defenseCard, Hero *attackingHero, Hero *defendingHero,
                            int defenderHealthBefore);
  void runSecondAfterCombatEvent(Fighter *attacker, Fighter *defender, Card *attackerCard,
                                  Card *defenseCard, Hero *attackingHero, Hero *defendingHero,
                                  Fighter *winner, Fighter *loser, int damage,
                                  int defenderHealthBefore);
  void finalizeCombat(Fighter *attacker, Fighter *defender, Card *attackerCard,
                       Card *defenseCard, Hero *attackingHero, Hero *defendingHero,
                       Fighter *winner, Fighter *loser, int damage, int defenderHealthBefore);

  Fighter *lastCombatWinner{nullptr};
  Fighter *lastCombatLoser{nullptr};

  bool matchOver{false};
  Hero *winningHero{nullptr};
  void checkForGameOver();

  void resetTurnState();

  std::vector<nlohmann::json> undoStack;
  void pushUndoCheckpoint();
  void clearUndoHistory();
  nlohmann::json serializeState() const;
  static std::unique_ptr<GameManager> buildFromState(const nlohmann::json &j);

  std::vector<int> allStartTileIds;
  void placeHeroesFrom(int heroIndex, std::vector<int> availableStartTileIds);
  Hero *heroAwaitingStartPlacement{nullptr};

  void placeSidekicksFrom(int heroIndex, std::size_t sidekickIndex);
  void finishSetup();
  Fighter *sidekickAwaitingPlacement{nullptr};

  void placeFogTokensFrom(int heroIndex, int fogIndex);
  Hero *fogTokenAwaitingHero{nullptr};

  void applyFatigue(Hero *hero);

  void checkFighterDeath(Fighter *fighter);
  void reapTheDead();

   
  void notifyDamage(Fighter *fighter, int healthBefore);

 
  bool heroHasCardInHand(const Hero *hero, const Card *card) const;

  bool performerAllows(const Card *card, const Fighter *actingFighter) const;

public:
  bool canPerform(const Card *card, const Fighter *actingFighter) const {
    return performerAllows(card, actingFighter);
  }

private:
  Hero *getNextHero(int fromPlayer) const;

  void triggerStartOfTurnAbility();

  void triggerDraculaBloodHarvest(Hero *hero);

  bool isAbilityProtected(const Hero *hero) const;
  bool isAbilityDisabled(const Hero *hero) const;
  std::vector<Hero *> disabledAbilityHeroes;

  std::unordered_map<Fighter *, bool> fogTileAtOwnerTurnStart;
  void snapshotFogTileAtTurnStart(Hero *hero);

  gameData buildGameData(Fighter *self, Fighter *target, Fighter *enemy,
                         Card *cardPlayed, Card *enemyCardPlayed,
                         const TypeOfEvent &event);

  void runCombatEvent(const TypeOfEvent &event, Fighter *cardOwner, Card *ownCard,
                      Fighter *opponent, Card *opponentCard);

public:
  GameManager(std::vector<std::unique_ptr<Hero>> heroes, std::unique_ptr<Map> map);
  ~GameManager();

  static std::unique_ptr<GameManager> createFromSelection(const std::string &mapName);

  bool isVsAI() const { return vsAI; } // true = player 2 is played by the AI

  static constexpr int kSaveSlotCount = 3;
  static std::string saveFilePath(int slot);
  static bool hasSave(int slot);

 bool saveGame(int slot) const;

 static std::unique_ptr<GameManager> loadGame(int slot);


  bool canUndo() const;
  std::unique_ptr<GameManager> undo();

  void startGame();

  int getCurrentTurn() const;
  Hero *getCurrentHero() const;
  Hero *getHero(int player) const;
  int getPlayerCount() const;
  Map &getMap();

  int getMovesRemaining(Fighter *fighter) const;

  int getMovesRemaining() const;

  void addObserver(IGameObserver *observer);
  void removeObserver(IGameObserver *observer);

  bool isAwaitingHeroStartPlacement() const { return heroAwaitingStartPlacement != nullptr; }
  Hero *getHeroAwaitingStartPlacement() const { return heroAwaitingStartPlacement; }

  bool isAwaitingSidekickPlacement() const { return sidekickAwaitingPlacement != nullptr; }
  Fighter *getSidekickAwaitingPlacement() const { return sidekickAwaitingPlacement; }

  bool isAwaitingFogTokenPlacement() const { return fogTokenAwaitingHero != nullptr; }
  Hero *getFogTokenAwaitingHero() const { return fogTokenAwaitingHero; }

  bool isGameOver() const;
  Hero *getWinner() const;

  bool endTurn();

  int getActionsRemaining() const;

  bool performManeuver();

  bool needsEndOfTurnDiscard() const;
  bool discardExcessCard(Card *card);

  bool moveFighter(Fighter *fighter, int tileId);

  bool moveThroughFog(Fighter *fighter, int destinationTileId);

  bool boostMovement(Card *card, Fighter *fighter = nullptr);

  bool canBoostMovement() const;

  void finishManeuver();
  bool isManeuverActive() const;

  bool startCombat(Card *attackCard, Fighter *attacker, Fighter *target);

  bool startCombat(Card *attackCard, Fighter *target);
  bool isCombatActive() const;
  Fighter *getCombatAttacker() const;
  Fighter *getCombatDefender() const;
  Card *getCombatAttackerCard() const;

  Hero *getCombatDefendingHero() const;

  bool resolveCombat(Card *defenseCard, int predictedAttackValue = -1);
  void cancelCombat();

  bool playCard(Card *card, Fighter *self, Fighter *target, Fighter *enemy);

  bool playCard(Card *card);

  bool cardNeedsTarget(const Card *card) const;
  std::vector<Fighter *> getValidTargetsForCard(const Card *card) const;

  bool disableAbility(Hero *hero);
  void enableAbility(Hero *hero);

  void requestTileChoice(std::vector<Tile *> options, std::function<void(Tile *)> onChosen,
                         Tile *explicitStayTile = nullptr);

  void requestTileChoice(Fighter *chooser, std::vector<Tile *> options,
                         std::function<void(Tile *)> onChosen, Tile *explicitStayTile = nullptr);
  void requestFighterChoice(std::vector<Fighter *> options, std::function<void(Fighter *)> onChosen);
  void requestFighterChoice(Fighter *chooser, std::vector<Fighter *> options,
                            std::function<void(Fighter *)> onChosen);
  void requestCardChoice(std::vector<Card *> options, std::function<void(Card *)> onChosen);
  void requestCardChoice(Fighter *chooser, std::vector<Card *> options, std::function<void(Card *)> onChosen);
  void requestEffectChoice(std::vector<std::string> labels, std::function<void(int)> onChosen);
  void requestEffectChoice(Fighter *chooser, std::vector<std::string> labels, std::function<void(int)> onChosen);

  bool isWaitingForTile() const;
  bool isWaitingForFighter() const;
  bool isWaitingForCard() const;
  bool isWaitingForEffectChoice() const;
  bool isWaitingForSelection() const;

  std::vector<Tile *> getValidTiles() const;
  std::vector<Fighter *> getValidFighters() const;
  std::vector<Card *> getValidCards() const;
  std::vector<std::string> getValidEffectChoiceLabels() const;

 Fighter *getPendingChooser() const { return pendingChooser; }

  Tile *getStayTileOption() const;

  void submitTile(Tile *chosen);
  void submitFighter(Fighter *chosen);
  void submitCard(Card *chosen);
  void submitEffectChoice(int index);
  void clearPending();
};