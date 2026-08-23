#pragma once
#include "raylib.h"
#include "view/scenes/Scene.h"
#include "controller/GameManager.h"
#include "engine/observer/observer.h"
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

class GameScene : public Scene, public IGameObserver
{
private:

  struct ActionButton
  {
    Rectangle rec;
    Texture2D icon;
    std::string title;
    std::string desc;
  };
  struct BoardTileView
  {
    int id;
    Vector2 rawPos;
    Vector2 pos;
    float radius;
    Color color;
    int badge;
    bool isPortal{false};
    Fighter *occupant{nullptr};

    Color color2{};
    bool hasSecondZone{false};
    Color color3{};
    bool hasThirdZone{false};

    int fogTokenCount{0};
  };
  struct HeroSummaryView

  {
    std::string name;
    int health{0};
    int maxHealth{0};
    int ownerPlayer{0};
    bool isCurrentTurn{false};
    std::vector<std::string> sidekickLines;
  };
  struct HandCardView
  {
    Card *card;
    std::string name;
    Texture2D *art{nullptr};
  };
  struct AbilityRow
  {
    std::string title;
    std::string desc;
  };

  Font titleFont;
  Font labelFont;
  Font smallFont;

  Texture2D iconSword;
  Texture2D iconShield;
  Texture2D iconBook;
  Texture2D iconCards;
  Texture2D map;

  Color bgDark;
  Color panelBg;
  Color gold;
  Color textLight;
  Color textMuted;

  std::unordered_map<std::string, Color> zoneColors;
  Color unknownZoneColor;
  Color portalRingColor;
  std::unique_ptr<GameManager> gameManager;

  std::vector<BoardTileView> tiles;
  std::vector<std::pair<int, int>> edges;
  std::unordered_map<int, int> idToIndex;

  std::vector<HandCardView> hand;
  std::vector<AbilityRow> abilities;
  std::string heroName;
  std::string mapName;
  int heroHealth{0};
  int heroMaxHealth{0};
  int heroShield{0};
  int cardsInDeck{0};
  int tokens{0};
  std::deque<std::string> eventLog;
  std::vector<HeroSummaryView> heroSummaries;
  std::vector<ActionButton> actionButtons;
  std::vector<Rectangle> handRects;
  int selectedAction{-1};
  int selectedCard{-1};

  bool moveModeActive{false};
  std::vector<int> legalMoveTiles;
  int hoveredMoveTile{-1};

  bool targetModeActive{false};
  Card *pendingTargetCard{nullptr};
  std::vector<Fighter *> legalTargets;
  int hoveredTargetTile{-1};

  int hoveredGenericTile{-1};

  int hoveredFighterChoiceTile{-1};
  Rectangle fighterChoiceDeclineRect{};

  bool pendingTargetIsAttack{false};

  bool attackModeActive{false};

  std::vector<int> attackRangeTiles;
  std::vector<Fighter *> attackRangeTargets;

  bool schemeModeActive{false};

  Fighter *activeFighter{nullptr};

  bool combatDefensePending{false};
  std::vector<Card *> defenseCardOptions;
  std::vector<Rectangle> defenseOptionRects;
  Rectangle noDefenseRect;
  Rectangle defensePanelRect;

  bool defenseHandRevealed{false};
  Rectangle revealHandRect;

  bool predictionPromptActive{false};
  Card *pendingPredictionCard{nullptr};
  std::vector<int> predictionValues;
  std::vector<Rectangle> predictionValueRects;
  Rectangle predictionCancelRect;
  Rectangle predictionPanelRect;

  std::vector<Rectangle> cardChoiceOptionRects;
  Rectangle cardChoiceDeclineRect;
  Rectangle cardChoicePanelRect;

  std::vector<Rectangle> effectChoiceOptionRects;
  Rectangle effectChoicePanelRect;

  bool handRevealActive{false};
  std::string handRevealOwnerName;
  std::vector<Card *> handRevealCards;
  std::vector<Rectangle> handRevealCardRects;
  Rectangle handRevealCloseRect;
  Rectangle handRevealPanelRect;

  float sw;
  float sh;

  Rectangle heroPanelRect;
  Rectangle opponentsPanelRect;
  Rectangle actionsPanelRect;
  Rectangle deckRect;
  Rectangle eventLogRect;
  Rectangle boardRect;

  static constexpr float kMapImageWidth = 1337.0f;
  static constexpr float kMapImageHeight = 866.0f;
  static constexpr float kBaseTileRadius = 34.0f;
  Rectangle mapDestRect{};
  Rectangle endTurnRect;
  Rectangle finishMovingRect;
  Rectangle stayPutRect;

  Rectangle resultMenuRect;

  bool matchOver{false};
  std::string winnerName;

  bool savePromptActive{false};
  Rectangle saveButtonRect{};
  Rectangle undoButtonRect{};
  Rectangle saveSlotPanelRect{};
  std::vector<Rectangle> saveSlotRects;
  std::vector<bool> saveSlotHasSave;
  Rectangle saveSlotCancelRect{};

  void refreshSaveSlotInfo();
  void openSavePrompt();
  void drawSavePrompt();
  void handleSaveMouse();

  void BuildBoardLayout();
  void refreshFromGameManager();
  void UpdateLayout();
  void drawBackground();
  void drawHeroPanel();
  void drawOpponentsPanel();
  void drawActionsPanel();
  void drawDeck();
  void drawBoard();
  void drawHand();
  void drawBottomBar();
  void drawEventLog();
  void drawResultScreen();
  void handleMouse();
  void handleKeyboard();
  void activateAction(int index);
  void playHandCard(int index);
  void tryBoostMovement(int index);
  void beginMovePicker();
  void cancelMovePicker();
  void tryMoveToTile(int tileId);
  void beginTargetPicker(Card *card);
  void cancelTargetPicker();
  void tryTargetFighter(Fighter *fighter);
  void handleGenericTilePick();
  void handleGenericFighterPick();

  Fighter *getActiveFighter() const;
  void trySelectActiveFighter(Fighter *fighter);

  Hero *getDefaultEnemyHero() const;

  void beginAttackPicker(Card *card);

  void updateAttackRangePreview();
  void clearAttackRangePreview();

  void beginDefensePrompt();
  void resolveDefense(Card *defenseCard, int predictedValue = -1);
  void drawDefensePrompt();
  void handleDefenseMouse();

  void beginPredictionPrompt(Card *defenseCard);
  void resolvePrediction(int guessedValue);
  void drawPredictionPrompt();
  void handlePredictionMouse();

  bool isCardChoicePromptActive() const;
  void drawCardChoicePrompt();
  void handleCardChoiceMouse();

  bool isEffectChoicePromptActive() const;
  void drawEffectChoicePrompt();
  void handleEffectChoiceMouse();

  bool isDiscardPromptActive() const;
  void drawDiscardPrompt();
  void handleDiscardMouse();

  bool isHandRevealPromptActive() const { return handRevealActive; }
  void drawHandRevealPrompt();
  void handleHandRevealMouse();

public:
  GameScene(AudioManager *, SceneManager *, TextureManager *, FontManager *);
  void Update() override;
  void Draw() override;
  void onEnter() override;
  ~GameScene() override;

  void onGameStarted() override;
  void onTurnEnded(int newCurrentPlayer) override;
  void onCardPlayed(Hero *player, Card *card) override;
  void onCardDiscarded(Hero *player, Card *card) override;
  void onFighterMoved(Fighter *fighter, int fromTileId, int toTileId) override;
  void onFighterDamaged(Fighter *fighter, int amount) override;
  void onManeuverPerformed(Hero *hero, bool deckWasEmpty) override;
  void onFighterRemoved(Fighter *fighter) override;
  void onCombatResolved(Fighter *attacker, Fighter *defender, Fighter *winner,
                        Fighter *loser, int damage) override;
  void onGameOver(Hero *winner) override;
  void onHandSeen(Hero *target, const std::vector<Card *> &hand) override;
};