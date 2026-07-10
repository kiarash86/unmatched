#pragma once                   // once per prg
#include "raylib.h"            // Rectangle/Vector2/Color/Font/Texture2D
#include "view/scenes/Scene.h" // father
#include <string>              // std::string
#include <unordered_map>       // tile id -> view position
#include <vector>              // containers
class GameScene : public Scene // in-match board (hero/map/actions/hand)
{
private:
  // NOTE: view/ui/Button.h's RayButton (rect/hoverColor/label/isHovered)
  // doesn't carry an icon or a second line of text, and MainScene doesn't
  // actually use it either -- it defines its own nested Button struct.
  // Following that same real precedent here instead of RayButton.
  struct ActionButton // one row in the actions panel, also mouse/key selectable
  {
    Rectangle rec;     // hit-box + draw position
    Texture2D icon;    // action icon
    std::string title; // Move / Attack / Defend / End Turn
    std::string desc;  // short description line
  };
  struct BoardTileView // one tile as drawn on screen (view-only, no game logic)
  {
    int id;       // must match Tile::getId() from the real model
    Vector2 pos;  // pixel position on the board panel
    float radius; // draw radius
    Color color;  // zone color
    int badge;    // number shown on the tile, 0 = none
  };
  struct HandCardView // one card drawn in the hand row
  {
    std::string name;     // card title
    std::string category; // move/attack/defend label
    int cost;             // number shown top-left of the card
  };
  struct AbilityRow // one row in the hero panel's ability list
  {
    std::string title; // ability name
    std::string desc;  // ability description
  };
  // FONTS
  Font titleFont; // hero name / panel titles
  Font labelFont; // ability/action titles
  Font smallFont; // descriptions, muted text
  // FONTS
  // TEXTURES
  Texture2D iconSword;  // attack icon
  Texture2D iconShield; // defend icon
  Texture2D iconBook;   // ability icon
  Texture2D iconCards;  // deck icon
  Texture2D map;        // map
  // TEXTURES
  // COLORS
  Color bgDark;         // base background
  Color panelBg;        // panel fill
  Color gold;           // accents / borders
  Color textLight;      // primary text
  Color textMuted;      // secondary text
  Color zoneGarden;     // outdoor tile color
  Color zoneRoom;       // brown room tile color
  Color zoneHallway;    // green hallway tile color
  Color zonePurpleRoom; // purple room tile color
  // COLORS
  // BOARD DATA (view-only, see BuildMockBoard/TODO below)
  std::vector<BoardTileView> tiles;       // tiles to draw
  std::vector<std::pair<int, int>> edges; // connections between tile ids
  std::unordered_map<int, int> idToIndex; // tile id -> index in `tiles`
  // BOARD DATA
  // GAME-FACING DATA (mock for now, replace with real Hero/Card/gameData)
  std::vector<HandCardView> hand;    // cards currently in hand
  std::vector<AbilityRow> abilities; // hero abilities
  std::string heroName;              // hero display name
  std::string mapName;               // map display name
  int heroHealth{0};                 // hero HP
  int heroShield{0};                 // hero defense value
  int cardsInDeck{0};                // remaining deck count
  int tokens{0};                     // generic token/resource count
  // GAME-FACING DATA
  std::vector<ActionButton> actionButtons; // Move/Attack/Defend/End Turn
  std::vector<Rectangle> handRects;        // hit-box per hand card
  int selectedAction{-1};                  // which action button is hovered
  int selectedCard{-1};                    // which hand card is hovered
  // LAYOUT
  float sw; // screen width
  float sh; // screen height
  // LAYOUT
  // POSITIONS
  Rectangle heroPanelRect;    // top-left hero panel
  Rectangle actionsPanelRect; // top-right actions panel
  Rectangle deckRect;         // deck widget under actions panel
  Rectangle boardRect;        // center board frame
  Rectangle endTurnRect;      // bottom-right end turn button
  // POSITIONS
  // FUNCS
  void BuildMockBoard(); // TODO: replace with a real Map/Tile -> tilePositions
                         // loader
  void UpdateLayout();   // recompute panel/board rects from sw/sh
  void drawBackground(); // base gradient fill
  void drawHeroPanel();  // portrait, hp/shield, abilities
  void drawActionsPanel(); // Move/Attack/Defend/End Turn rows
  void drawDeck();         // deck widget
  void drawBoard();        // tiles + edges
  void drawHand();         // hand row of cards
  void drawBottomBar();    // deck/token counters, title strip, end turn button
  void handleMouse();      // hover/click on action buttons + hand cards
  void handleKeyboard();   // left/right + enter navigation for hand/actions
  void activateAction(int index); // Move/Attack/Defend/End Turn was chosen
  // FUNCS
public:
  // FUNCS
  GameScene(AudioManager *, SceneManager *, TextureManager *,
            FontManager *); // constructor
  void Update() override;   // update
  void Draw() override;     // drawing
  void onEnter() override;  // entering scene
  ~GameScene() override;    // no texture/font ownership (managers own them)
  // FUNCS
};