#include "view/scenes/GameScene.h"
#include "controller/PlayerSelectionManager.h"
#include "controller/SceneManager.h"

GameScene::GameScene(AudioManager *audioManager, SceneManager *sceneManager,
                     TextureManager *textureManager, FontManager *fontManager)
    : Scene(audioManager, sceneManager, textureManager, fontManager) {

  titleFont = fontManager->getFont(FontID::CinzelSemiBold, 32);
  labelFont = fontManager->getFont(FontID::CormorantGaramondMedium, 20);
  smallFont = fontManager->getFont(FontID::CormorantGaramondRegular, 16);

  map = textureManager->getTexture(TextureID::manorMap);
  iconSword = textureManager->getTexture(TextureID::iconSword);
  iconShield = textureManager->getTexture(TextureID::iconShield);
  iconBook = textureManager->getTexture(TextureID::iconBook);
  iconCards = textureManager->getTexture(TextureID::iconCards);
  bgDark = {15, 15, 15, 255};
  panelBg = {22, 20, 18, 235};
  gold = {212, 175, 55, 255};
  textLight = {225, 220, 210, 255};
  textMuted = {150, 145, 135, 255};

  zoneGarden = {70, 95, 105, 255};
  zoneRoom = {110, 78, 55, 255};
  zoneHallway = {70, 110, 80, 255};
  zonePurpleRoom = {110, 80, 130, 255};

  // TODO: replace this whole mock block with real data pulled from
  // Hero / gameData once those layers compile.
  heroName = "Sherlock Holmes";
  heroHealth = 17;
  heroShield = 2;
  mapName = "Baskerville Manor";
  cardsInDeck = 5;
  tokens = 2;

  abilities = {
      {"Deductive Genius", "After you move, draw 1 card."},
      {"Elementary", "When you attack, add 1 to your combat value."},
      {"Prepared", "Reduce the damage you take by 1."},
  };

  hand = {
      {"Observation", "Move", 2},   {"Logic", "Move", 3},
      {"Violin Case", "Attack", 3}, {"Pipe", "Attack", 2},
      {"Inference", "Defend", 1},
  };

  actionButtons = {
      {{}, iconSword, "Attack", "Attack an adjacent opponent."},
      {{}, iconBook, "Move", "Move up to 2 spaces."},
      {{}, iconShield, "Defend", "Prepare a defense card."},
      {{}, iconCards, "End Turn", "Discard down to 5 cards and end your turn."},
  };

  BuildMockBoard();

  sw = (float)GetScreenWidth();
  sh = (float)GetScreenHeight();
  UpdateLayout();
}

GameScene::~GameScene() {
  // AudioManager/TextureManager/FontManager own every asset we used above,
  // so there's nothing for this scene to unload itself.
}

void GameScene::onEnter() {
  selectedAction = -1;
  selectedCard = -1;

   const auto &pickedPlayers = PlayerSelectionManager::instance().getPlayers();
  if (!pickedPlayers.empty()) {
    heroName = pickedPlayers[0].getHeroName();
  }
}


void GameScene::BuildMockBoard() {
  tiles.clear();
  edges.clear();
  idToIndex.clear();

  auto add = [&](int id, float x, float y, float r, Color c, int badge = 0) {
    idToIndex[id] = (int)tiles.size();
    tiles.push_back({id, {x, y}, r, c, badge});
  };

  add(0, 260, 110, 34, zoneGarden, 1);
  add(1, 420, 100, 34, zoneGarden);
  add(2, 250, 230, 34, zoneGarden);
  add(3, 400, 230, 34, zoneGarden, 2);
  add(4, 390, 350, 34, zoneGarden);
  add(5, 280, 420, 34, zoneGarden, 4);
  add(6, 430, 470, 34, zoneHallway);

  add(7, 230, 560, 34, zoneGarden, 1);
  add(8, 340, 610, 34, zoneGarden);
  add(9, 460, 620, 34, zoneGarden);
  add(10, 580, 630, 34, zoneGarden);
  add(11, 700, 640, 34, zoneHallway, 3);
  add(12, 820, 640, 34, zoneGarden);
  add(13, 940, 630, 34, zoneGarden);
  add(14, 1060, 610, 34, zonePurpleRoom);
  add(15, 950, 560, 34, zoneGarden);

  add(16, 560, 90, 34, zoneRoom);
  add(17, 700, 80, 34, zoneRoom);
  add(18, 630, 190, 34, zoneRoom);

  add(19, 950, 90, 34, zoneRoom);
  add(20, 1080, 100, 34, zoneRoom);
  add(21, 1015, 190, 34, zoneRoom);

  add(22, 690, 330, 34, zoneHallway);
  add(23, 800, 330, 34, zoneHallway, 1);
  add(24, 680, 430, 34, zoneHallway);
  add(25, 800, 440, 34, zoneHallway);

  add(26, 1020, 340, 34, zonePurpleRoom);
  add(27, 940, 440, 34, zonePurpleRoom);
  add(28, 1060, 450, 34, zonePurpleRoom);

  edges = {
      {0, 1},   {0, 2},   {1, 3},   {2, 3},   {2, 4},   {3, 4},
      {4, 5},   {5, 6},   {6, 24},  {5, 7},   {7, 8},   {8, 9},
      {9, 10},  {10, 11}, {11, 12}, {12, 13}, {13, 14}, {14, 15},
      {15, 28}, {16, 17}, {16, 18}, {17, 18}, {18, 22}, {19, 20},
      {19, 21}, {20, 21}, {21, 26}, {22, 23}, {22, 24}, {23, 25},
      {23, 26}, {24, 25}, {25, 27}, {26, 27}, {26, 28}, {27, 28},
  };
}

void GameScene::UpdateLayout() {
  float margin = 20.0f;
  float panelW = 260.0f;

  heroPanelRect = {margin, margin, panelW, 260.0f};
  actionsPanelRect = {sw - panelW - margin, margin, panelW, 260.0f};
  deckRect = {actionsPanelRect.x + 65, actionsPanelRect.y + 280, 130, 170};
  boardRect = {margin + panelW + 20, margin, sw - 2 * panelW - 2 * margin - 40,
               sh - 260};
  endTurnRect = {sw - 170, sh - 60, 150, 44};

  float rowY = actionsPanelRect.y + 46;
  for (auto &btn : actionButtons) {
    btn.rec = {actionsPanelRect.x + 10, rowY, actionsPanelRect.width - 20, 40};
    rowY += 50;
  }

  handRects.clear();
  float cardW = 130, cardH = 150, gap = 12;
  float totalW = hand.size() * cardW + (hand.size() - 1) * gap;
  float startX = sw / 2 - totalW / 2;
  float handY = sh - 190;
  for (size_t i = 0; i < hand.size(); i++) {
    handRects.push_back({startX + i * (cardW + gap), handY, cardW, cardH});
  }
}

void GameScene::drawBackground() {
  ClearBackground(bgDark);
  DrawRectangleGradientV(0, 0, (int)sw, (int)sh, Color{40, 32, 22, 255},
                         bgDark);
}

void GameScene::drawHeroPanel() {
  DrawRectangleRounded(heroPanelRect, 0.06f, 8, panelBg);
  DrawRectangleRoundedLines(heroPanelRect, 0.06f, 8, 2, gold);

  DrawTextEx(titleFont, heroName.c_str(),
             {heroPanelRect.x + 14, heroPanelRect.y + 12}, 20, 1, textLight);

  Rectangle portrait = {heroPanelRect.x + 10, heroPanelRect.y + 40, 90, 90};
  DrawRectangleRec(portrait, Color{35, 30, 25, 255});
  DrawRectangleLinesEx(portrait, 2, gold);

  float statY = heroPanelRect.y + 140;
  DrawTextEx(smallFont, TextFormat("HP %d", heroHealth),
             {heroPanelRect.x + 14, statY}, 18, 1, RED);
  DrawTextEx(smallFont, TextFormat("DEF %d", heroShield),
             {heroPanelRect.x + 100, statY}, 18, 1, SKYBLUE);

  float abY = heroPanelRect.y + 178;
  for (auto &ab : abilities) {
    DrawTextEx(labelFont, ab.title.c_str(), {heroPanelRect.x + 14, abY}, 16, 1,
               gold);
    DrawTextEx(smallFont, ab.desc.c_str(), {heroPanelRect.x + 14, abY + 16}, 13,
               1, textMuted);
    abY += 42;
  }
}

void GameScene::drawActionsPanel() {
  DrawRectangleRounded(actionsPanelRect, 0.06f, 8, panelBg);
  DrawRectangleRoundedLines(actionsPanelRect, 0.06f, 8, 2, gold);
  DrawTextEx(titleFont, "ACTIONS",
             {actionsPanelRect.x + 14, actionsPanelRect.y + 12}, 20, 1,
             textLight);

  for (size_t i = 0; i < actionButtons.size(); i++) {
    auto &a = actionButtons[i];
    bool hovered = ((int)i == selectedAction);

    if (hovered)
      DrawRectangleRounded(a.rec, 0.2f, 6, Color{40, 45, 60, 220});

    DrawTextureEx(a.icon, {a.rec.x, a.rec.y}, 0, 20.0f / a.icon.width, WHITE);
    DrawTextEx(labelFont, a.title.c_str(), {a.rec.x + 32, a.rec.y - 2}, 16, 1,
               hovered ? gold : textLight);
    DrawTextEx(smallFont, a.desc.c_str(), {a.rec.x + 32, a.rec.y + 14}, 12, 1,
               textMuted);
  }
}

void GameScene::drawDeck() {
  DrawRectangleRounded(deckRect, 0.08f, 8, Color{45, 40, 35, 255});
  DrawRectangleRoundedLines(deckRect, 0.08f, 8, 2, Color{90, 85, 75, 255});
  DrawCircleLines((int)(deckRect.x + deckRect.width / 2),
                  (int)(deckRect.y + deckRect.height / 2), 40,
                  Color{140, 130, 110, 180});
  DrawTextEx(labelFont, "DECK",
             {deckRect.x + 38, deckRect.y + deckRect.height / 2 - 8}, 16, 1,
             textMuted);
}

void GameScene::drawBoard() {
  DrawRectangleRounded(boardRect, 0.02f, 6, Color{18, 25, 28, 255});

  for (auto &e : edges) {
    Vector2 a = tiles[idToIndex.at(e.first)].pos;
    Vector2 b = tiles[idToIndex.at(e.second)].pos;
    a.x += boardRect.x;
    a.y += boardRect.y;
    b.x += boardRect.x;
    b.y += boardRect.y;
    DrawLineEx(a, b, 6, Color{20, 18, 15, 255});
  }

  for (auto &t : tiles) {
    Vector2 c = {t.pos.x + boardRect.x, t.pos.y + boardRect.y};
    DrawCircleV(c, t.radius, t.color);
    DrawCircleLines((int)c.x, (int)c.y, t.radius, Color{0, 0, 0, 90});
    if (t.badge > 0) {
      Vector2 b = {c.x - t.radius + 6, c.y - t.radius + 6};
      DrawCircleV(b, 11, WHITE);
      DrawCircleLines((int)b.x, (int)b.y, 11, DARKGRAY);
      DrawText(TextFormat("%d", t.badge), (int)b.x - 4, (int)b.y - 7, 14,
               BLACK);
    }
  }
}

void GameScene::drawHand() {
  for (size_t i = 0; i < hand.size(); i++) {
    Rectangle &r = handRects[i];
    bool hovered = ((int)i == selectedCard);

    DrawRectangleRounded(r, 0.08f, 6, Color{30, 27, 22, 255});
    DrawRectangleRoundedLines(r, 0.08f, 6, 2,
                              hovered ? gold : Color{110, 100, 80, 255});

    DrawCircleV({r.x + 16, r.y + 14}, 13, gold);
    DrawText(TextFormat("%d", hand[i].cost), (int)r.x + 11, (int)r.y + 4, 18,
             BLACK);

    DrawTextEx(labelFont, hand[i].name.c_str(), {r.x + 10, r.y + r.height - 40},
               14, 1, textLight);
    DrawTextEx(smallFont, hand[i].category.c_str(),
               {r.x + 10, r.y + r.height - 22}, 12, 1, textMuted);
  }
}

void GameScene::drawBottomBar() {
  DrawRectangleRounded({20, sh - 90, 90, 60}, 0.2f, 6, panelBg);
  DrawText(TextFormat("%d", cardsInDeck), 55, (int)sh - 70, 22, textLight);

  DrawRectangleRounded({120, sh - 90, 90, 60}, 0.2f, 6, panelBg);
  DrawCircleV({160, sh - 60}, 14, Color{150, 20, 20, 255});
  DrawText(TextFormat("%d", tokens), 178, (int)sh - 70, 22, textLight);

  DrawText("UNMATCHED", 20, (int)sh - 22, 16, textMuted);
  DrawTextEx(smallFont, mapName.c_str(), {140, sh - 22}, 16, 1, gold);

  DrawRectangleRounded(endTurnRect, 0.15f, 6, Color{25, 55, 90, 255});
  DrawRectangleRoundedLines(endTurnRect, 0.15f, 6, 2, SKYBLUE);
  DrawText("END TURN", (int)endTurnRect.x + 22, (int)endTurnRect.y + 12, 18,
           WHITE);
}

void GameScene::handleMouse() {
  Vector2 mouse = GetMousePosition();

  selectedAction = -1;
  for (size_t i = 0; i < actionButtons.size(); i++) {
    if (CheckCollisionPointRec(mouse, actionButtons[i].rec)) {
      selectedAction = (int)i;
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        activateAction(selectedAction);
      break;
    }
  }

  selectedCard = -1;
  for (size_t i = 0; i < handRects.size(); i++) {
    if (CheckCollisionPointRec(mouse, handRects[i])) {
      selectedCard = (int)i;
      break;
    }
  }

  if (CheckCollisionPointRec(mouse, endTurnRect) &&
      IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    activateAction((int)actionButtons.size() - 1); // End Turn is last
  }
}

void GameScene::handleKeyboard() {
  if (IsKeyPressed(KEY_RIGHT)) {
    selectedAction = (selectedAction + 1) % (int)actionButtons.size();
  }
  if (IsKeyPressed(KEY_LEFT)) {
    selectedAction = (selectedAction - 1 + (int)actionButtons.size()) %
                     (int)actionButtons.size();
  }
  if (IsKeyPressed(KEY_ENTER) && selectedAction >= 0) {
    activateAction(selectedAction);
  }
}

void GameScene::activateAction(int index) {
  // TODO: wire these into the real turn/combat logic once the engine
  // layer compiles; for now this only exists so hover/click has an effect.
  if (index < 0 || index >= (int)actionButtons.size())
    return;
  if (actionButtons[index].title == "End Turn") {
    // scene->changeScene(...); // go to opponent's turn / next phase
  }
}

void GameScene::Update() {
  handleMouse();
  handleKeyboard();
}

void GameScene::Draw() {
  sw = (float)GetScreenWidth();
  sh = (float)GetScreenHeight();
  UpdateLayout();

  drawBackground();
  drawHeroPanel();
  drawActionsPanel();
  drawDeck();
  drawBoard();

  //DrawTexturePro(map, {0, 0, (float)map.width, (float)map.height}, boardRect, {0, 0}, 0, WHITE);

  drawHand();
  drawBottomBar();
}