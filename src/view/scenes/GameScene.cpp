#include "view/scenes/GameScene.h"
#include "controller/PlayerSelectionManager.h"
#include "controller/SceneManager.h"
#include "libraries/magic_enum.hpp"
#include "model/deck.h"
#include "model/sidekick.h"
#include "model/typeOfCard.h"
#include <algorithm>
#include <cmath>
namespace {
const std::string kDefaultMap = "baskervilleManor";

std::string categoryLabel(TypeOfCard type) {
  switch (type) {
  case TypeOfCard::attack:
    return "Attack";
  case TypeOfCard::def:
    return "Defend";
  case TypeOfCard::event:
    return "Event";

  case TypeOfCard::multipurpose:
    return "Atk/Def";
  default:
    return "Event";
  }
}

std::string timingLabel(TypeOfEvent event) {
  switch (event) {
  case TypeOfEvent::during_combat:
    return "During Combat";
  case TypeOfEvent::before_combat:
    return "Immediately";
  case TypeOfEvent::after_combat:
    return "After Combat";
  default:
    return "";
  }
}

void pushLog(std::deque<std::string> &log, const std::string &line) {
  log.push_back(line);
  while (log.size() > 5)
    log.pop_front();
}

// Returns a rectangle of size panelW x panelH centered on the sw x sh screen.
// Used by every popup/prompt panel (defense, prediction, card choice, hand
// reveal) so they all share one centering formula.
Rectangle centeredPanel(float sw, float sh, float panelW, float panelH) {
  return {sw / 2 - panelW / 2, sh / 2 - panelH / 2, panelW, panelH};
}

// Lays out `count` stacked option rows of height rowH inside `panel`,
// starting `topOffset` px below the panel's top, each row spanning
// panel.x+20 .. panel.width-40 with a 10px gap between rows. If `outRects`
// is non-null, each row's rectangle is appended to it. Returns the Y
// coordinate immediately after the last row (where a trailing button/rect,
// e.g. "no defense" or "close", can be placed).
float layoutStackedRows(const Rectangle &panel, float topOffset, float rowH,
                        int count, std::vector<Rectangle> *outRects) {
  float y = panel.y + topOffset;
  for (int i = 0; i < count; i++) {
    if (outRects)
      outRects->push_back({panel.x + 20, y, panel.width - 40, rowH});
    y += rowH + 10.0f;
  }
  return y;
}
} // namespace

GameScene::GameScene(AudioManager *audioManager, SceneManager *sceneManager,
                     TextureManager *textureManager, FontManager *fontManager)
    : Scene(audioManager, sceneManager, textureManager, fontManager) {

  titleFont = fontManager->getFont(FontID::t, 38);
  labelFont = fontManager->getFont(FontID::t, 24);
  smallFont = fontManager->getFont(FontID::t, 20);

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

  zoneColors = {
      {"light-blue", Color{90, 140, 175, 130}},
      {"dark-blue", Color{55, 80, 140, 130}},
      {"brown", Color{110, 78, 55, 130}},
      {"green", Color{70, 110, 80, 130}},
      {"yellow", Color{165, 140, 55, 130}},
      {"purple", Color{110, 80, 130, 130}},
      {"grey", Color{95, 95, 100, 130}},
  };
  unknownZoneColor = {80, 80, 80, 130};
  portalRingColor = {190, 140, 230, 230};

  actionButtons = {
      {{},
       iconSword,
       "Attack",
       "Choose an Attack/Versatile card, then a target."},
      {{},
       iconBook,
       "Maneuver",
       "Draw a card (mandatory), then optionally move."},
      {{}, iconShield, "Scheme", "Play a Scheme (event) card from your hand."},
      {{}, iconCards, "End Turn", "Discard down to 7, then end your turn."},
  };

  gameManager = GameManager::createFromSelection(kDefaultMap);
  gameManager->addObserver(this);
  gameManager->startGame();

  BuildBoardLayout();
  refreshFromGameManager();

  sw = (float)GetScreenWidth();
  sh = (float)GetScreenHeight();
  UpdateLayout();
}

GameScene::~GameScene() {
  if (gameManager) {
    gameManager->removeObserver(this);
  }
}

void GameScene::onEnter() {
  selectedAction = -1;
  selectedCard = -1;
  cancelMovePicker();
  cancelTargetPicker();
  attackModeActive = false;
  schemeModeActive = false;
  combatDefensePending = false;
  defenseCardOptions.clear();
  defenseHandRevealed = false;
  predictionPromptActive = false;
  pendingPredictionCard = nullptr;
  matchOver = false;
  winnerName.clear();
  handRevealActive = false;
  handRevealCards.clear();
  refreshFromGameManager();
}

void GameScene::refreshFromGameManager() {
  Hero *hero = gameManager->getCurrentHero();
  if (!hero)
    return;

  if (!activeFighter || !activeFighter->isAlive() ||
      activeFighter->getOwnerPlayer() != hero->getOwnerPlayer()) {
    activeFighter = hero;
  }

  heroName = hero->getName();
  heroHealth = hero->getHealth();
  heroShield = 0;

  mapName = gameManager->getMap().getName();

  abilities.clear();
  if (heroName == "Dracula") {
    abilities.push_back(
        {"Blood Harvest",
         "At the start of your turn, you may deal 1 damage to a fighter "
         "adjacent to Dracula (including one of your own Sisters). If you "
         "do, draw 1 card."});
  } else if (heroName == "SherlockHolms") {
    abilities.push_back(
        {"Unyielding Deduction",
         "Card effects cannot cancel or disable the abilities of Sherlock "
         "Holmes or Dr. Watson."});
  }

  hand.clear();
  if (hero->getDeck()) {
    cardsInDeck = hero->getDeck()->drawPileCount();
    for (Card *card : hero->getDeck()->getHand()) {

      bool hasStat = card->getCardType() != TypeOfCard::event;
      int statValue = card->getCardType() == TypeOfCard::def
                          ? card->getDefStat()
                          : card->getAttackStat();
      Texture2D *art = nullptr;
      if (auto texId =
              magic_enum::enum_cast<TextureID>((heroName, card->getName()))) {
        art = &texture->getTexture(*texId);
      }

      hand.push_back({card, card->getName(), categoryLabel(card->getCardType()),
                      card->getBoost(), hasStat, statValue,
                      timingLabel(card->getEventType()), art});
    }
  } else {
    cardsInDeck = 0;
  }

  for (auto &t : tiles) {
    Fighter *occ = gameManager->getMap().getFighterAt(t.id);
    t.occupant = occ;
    t.badge = occ ? 1 : 0;
  }

  heroSummaries.clear();
  for (int p = 0; p < gameManager->getPlayerCount(); p++) {
    Hero *h = gameManager->getHero(p);
    if (!h)
      continue;

    HeroSummaryView summary;
    summary.name = h->getName();
    summary.health = h->getHealth();
    summary.maxHealth = h->getMaxHealth();
    summary.ownerPlayer = p;
    summary.isCurrentTurn = (h == hero);
    for (auto &sk : h->getSidekicks()) {
      summary.sidekickLines.push_back(
          TextFormat("%s  HP %d/%d", sk->getName().c_str(), sk->getHealth(),
                     sk->getMaxHealth()));
    }
    heroSummaries.push_back(std::move(summary));
  }

  handRects.clear();
}

void GameScene::BuildBoardLayout() {
  tiles.clear();
  edges.clear();
  idToIndex.clear();

  Map &board = gameManager->getMap();
  const auto &realTiles = board.getTiles();

  static const std::vector<std::string> zonePriority = {
      "light-blue", "dark-blue", "brown", "green", "yellow", "purple", "grey"};

  for (auto &[id, tile] : realTiles) {
    if (!tile)
      continue;

    Color color = unknownZoneColor;
    Color color2{};
    Color color3{};
    bool foundFirst = false;
    bool hasSecondZone = false;
    bool hasThirdZone = false;
    for (const auto &zoneName : zonePriority) {
      if (!tile->getZones().count(zoneName))
        continue;
      if (!foundFirst) {
        color = zoneColors.at(zoneName);
        foundFirst = true;
      } else if (!hasSecondZone) {
        color2 = zoneColors.at(zoneName);
        hasSecondZone = true;
      } else {
        color3 = zoneColors.at(zoneName);
        hasThirdZone = true;
        break;
      }
    }

    Vector2D p = tile->getPosition();
    Vector2 raw{(float)p.x, (float)p.y};
    idToIndex[id] = (int)tiles.size();
    tiles.push_back({id, raw, raw, kBaseTileRadius, color, 0, tile->isPortal(),
                     nullptr, color2, hasSecondZone, color3, hasThirdZone});
  }

  for (auto &[id, tile] : realTiles) {
    if (!tile)
      continue;
    for (int neighborId : tile->getNeighbors()) {
      if (id < neighborId)
        edges.push_back({id, neighborId});
    }
  }
}

void GameScene::UpdateLayout() {
  float margin = 20.0f;
  float panelW = 260.0f;

  int ownSidekickLineCount = 0;
  for (auto &s : heroSummaries) {
    if (s.isCurrentTurn)
      ownSidekickLineCount = (int)s.sidekickLines.size();
  }
  heroPanelRect = {margin, margin, panelW,
                   260.0f + ownSidekickLineCount * 18.0f};

  int otherHeroCount = std::max(0, (int)heroSummaries.size() - 1);
  int sidekickLineCount = 0;
  for (auto &s : heroSummaries) {
    if (!s.isCurrentTurn)
      sidekickLineCount += (int)s.sidekickLines.size();
  }
  float opponentsPanelH =
      34.0f + otherHeroCount * 30.0f + sidekickLineCount * 18.0f;
  opponentsPanelRect = {margin, heroPanelRect.y + heroPanelRect.height + 14,
                        panelW, opponentsPanelH};
  eventLogRect = {margin, opponentsPanelRect.y + opponentsPanelRect.height + 14,
                  panelW, 172.0f};
  actionsPanelRect = {sw - panelW - margin, margin, panelW, 300.0f};
  deckRect = {actionsPanelRect.x + 65, actionsPanelRect.y + 320, 130, 170};
  boardRect = {margin + panelW + 20, margin, sw - 2 * panelW - 2 * margin - 40,
               sh - 370};

  {
    float scale = std::min(boardRect.width / kMapImageWidth,
                           boardRect.height / kMapImageHeight);
    float destW = kMapImageWidth * scale;
    float destH = kMapImageHeight * scale;
    mapDestRect = {boardRect.x + (boardRect.width - destW) / 2,
                   boardRect.y + (boardRect.height - destH) / 2, destW, destH};

    float offsetX = mapDestRect.x - boardRect.x;
    float offsetY = mapDestRect.y - boardRect.y;
    for (auto &t : tiles) {
      t.pos = {offsetX + t.rawPos.x * scale, offsetY + t.rawPos.y * scale};
      t.radius = kBaseTileRadius * scale;
    }
  }

  endTurnRect = {sw - 170, sh - 60, 150, 44};
  finishMovingRect = {sw - 340, sh - 60, 150, 44};
  stayPutRect = {sw - 340, sh - 60, 150, 44};
  resultMenuRect = {sw / 2 - 110, sh / 2 + 60, 220, 48};

  float rowY = actionsPanelRect.y + 50;
  for (auto &btn : actionButtons) {
    btn.rec = {actionsPanelRect.x + 10, rowY, actionsPanelRect.width - 20, 46};
    rowY += 58;
  }

  handRects.clear();
  float cardW = 210, cardH = 260, gap = 16;
  float totalW =
      hand.size() * cardW + (hand.empty() ? 0 : (hand.size() - 1) * gap);
  float startX = sw / 2 - totalW / 2;
  float handY = sh - cardH - 40;
  for (size_t i = 0; i < hand.size(); i++) {
    handRects.push_back({startX + i * (cardW + gap), handY, cardW, cardH});
  }

  float defCardH = 46.0f;
  float defPanelH =
      110.0f + (defenseCardOptions.size() + 1) * (defCardH + 10.0f);
  float defPanelW = 360.0f;
  defensePanelRect = centeredPanel(sw, sh, defPanelW, defPanelH);
  defenseOptionRects.clear();
  float optY =
      layoutStackedRows(defensePanelRect, 90.0f, defCardH,
                        (int)defenseCardOptions.size(), &defenseOptionRects);
  noDefenseRect = {defensePanelRect.x + 20, optY, defPanelW - 40, defCardH};
  revealHandRect = {defensePanelRect.x + 20, defensePanelRect.y + 90.0f,
                    defPanelW - 40,
                    (optY + defCardH) - (defensePanelRect.y + 90.0f)};

  predictionValues.clear();
  for (int v = 0; v <= 9; v++)
    predictionValues.push_back(v);

  float predBtn = 56.0f, predGap = 10.0f;
  int perRow = 5;
  int rows = (int)((predictionValues.size() + perRow - 1) / perRow);
  float predPanelW = perRow * predBtn + (perRow - 1) * predGap + 40.0f;
  float predPanelH = 110.0f + rows * (predBtn + predGap) + 56.0f;
  predictionPanelRect = centeredPanel(sw, sh, predPanelW, predPanelH);

  predictionValueRects.clear();
  for (size_t i = 0; i < predictionValues.size(); i++) {
    int row = (int)(i / perRow), col = (int)(i % perRow);
    predictionValueRects.push_back(
        {predictionPanelRect.x + 20 + col * (predBtn + predGap),
         predictionPanelRect.y + 90 + row * (predBtn + predGap), predBtn,
         predBtn});
  }
  predictionCancelRect = {predictionPanelRect.x + 20,
                          predictionPanelRect.y + 90 +
                              rows * (predBtn + predGap) + 10,
                          predPanelW - 40, 40};

  std::vector<Card *> cardChoiceOptions = gameManager->isWaitingForCard()
                                              ? gameManager->getValidCards()
                                              : std::vector<Card *>{};
  bool cardChoiceHasDecline =
      std::find(cardChoiceOptions.begin(), cardChoiceOptions.end(), nullptr) !=
      cardChoiceOptions.end();
  int realCardChoiceCount =
      (int)cardChoiceOptions.size() - (cardChoiceHasDecline ? 1 : 0);

  float choiceCardH = 46.0f;
  float choicePanelH =
      110.0f + (realCardChoiceCount + (cardChoiceHasDecline ? 1 : 0)) *
                   (choiceCardH + 10.0f);
  float choicePanelW = 360.0f;
  cardChoicePanelRect = centeredPanel(sw, sh, choicePanelW, choicePanelH);
  cardChoiceOptionRects.clear();
  float choiceOptY =
      layoutStackedRows(cardChoicePanelRect, 90.0f, choiceCardH,
                        realCardChoiceCount, &cardChoiceOptionRects);
  if (cardChoiceHasDecline) {
    cardChoiceDeclineRect = {cardChoicePanelRect.x + 20, choiceOptY,
                             choicePanelW - 40, choiceCardH};
  } else {
    cardChoiceDeclineRect = {0, 0, 0, 0};
  }

  float revealCardH = 46.0f;
  float revealPanelH = 150.0f + handRevealCards.size() * (revealCardH + 10.0f);
  float revealPanelW = 380.0f;
  handRevealPanelRect = centeredPanel(sw, sh, revealPanelW, revealPanelH);
  handRevealCardRects.clear();
  float revealY =
      layoutStackedRows(handRevealPanelRect, 90.0f, revealCardH,
                        (int)handRevealCards.size(), &handRevealCardRects);
  handRevealCloseRect = {handRevealPanelRect.x + 20, revealY, revealPanelW - 40,
                         40};
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
             {heroPanelRect.x + 14, heroPanelRect.y + 12}, 26, 1, textLight);
  DrawTextEx(smallFont,
             TextFormat("(your turn -- %d action%s left)",
                        gameManager->getActionsRemaining(),
                        gameManager->getActionsRemaining() == 1 ? "" : "s"),
             {heroPanelRect.x + 14, heroPanelRect.y + 38}, 16, 1, gold);

  Rectangle portrait = {heroPanelRect.x + 10, heroPanelRect.y + 46, 90, 90};
  DrawRectangleRec(portrait, Color{35, 30, 25, 255});
  DrawRectangleLinesEx(portrait, 2, gold);

  float statY = heroPanelRect.y + 148;
  DrawTextEx(smallFont, TextFormat("HP %d", heroHealth),
             {heroPanelRect.x + 14, statY}, 22, 1, RED);
  DrawTextEx(smallFont, TextFormat("DEF %d", heroShield),
             {heroPanelRect.x + 110, statY}, 22, 1, SKYBLUE);

  float skY = statY + 28;
  for (auto &s : heroSummaries) {
    if (!s.isCurrentTurn)
      continue;
    for (auto &line : s.sidekickLines) {
      DrawTextEx(smallFont, line.c_str(), {heroPanelRect.x + 14, skY}, 16, 1,
                 textMuted);
      skY += 18;
    }
    break;
  }

  float abY = skY + 12;
  for (auto &ab : abilities) {
    DrawTextEx(labelFont, ab.title.c_str(), {heroPanelRect.x + 14, abY}, 20, 1,
               gold);
    DrawTextEx(smallFont, ab.desc.c_str(), {heroPanelRect.x + 14, abY + 20}, 16,
               1, textMuted);
    abY += 50;
  }
}

void GameScene::drawOpponentsPanel() {
  DrawRectangleRounded(opponentsPanelRect, 0.06f, 8, panelBg);
  DrawRectangleRoundedLines(opponentsPanelRect, 0.06f, 8, 2,
                            Color{110, 60, 60, 255});

  float y = opponentsPanelRect.y + 10;
  for (auto &s : heroSummaries) {
    if (s.isCurrentTurn)
      continue;

    Color ownerTint =
        (s.ownerPlayer % 2 == 0) ? SKYBLUE : Color{220, 120, 120, 255};
    DrawCircleV({opponentsPanelRect.x + 16, y + 9}, 6, ownerTint);
    DrawTextEx(labelFont, s.name.c_str(), {opponentsPanelRect.x + 30, y}, 20, 1,
               textLight);
    DrawTextEx(smallFont, TextFormat("HP %d/%d", s.health, s.maxHealth),
               {opponentsPanelRect.x + opponentsPanelRect.width - 90, y}, 18, 1,
               RED);
    y += 32;

    for (auto &line : s.sidekickLines) {
      DrawTextEx(smallFont, line.c_str(), {opponentsPanelRect.x + 40, y}, 16, 1,
                 textMuted);
      y += 22;
    }
  }
}

void GameScene::drawActionsPanel() {
  DrawRectangleRounded(actionsPanelRect, 0.06f, 8, panelBg);
  DrawRectangleRoundedLines(actionsPanelRect, 0.06f, 8, 2, gold);
  DrawTextEx(titleFont, "ACTIONS",
             {actionsPanelRect.x + 14, actionsPanelRect.y + 12}, 26, 1,
             textLight);

  for (size_t i = 0; i < actionButtons.size(); i++) {
    auto &a = actionButtons[i];
    bool hovered = ((int)i == selectedAction);

    if (hovered)
      DrawRectangleRounded(a.rec, 0.2f, 6, Color{40, 45, 60, 220});

    DrawTextureEx(a.icon, {a.rec.x, a.rec.y}, 0, 24.0f / a.icon.width, WHITE);
    DrawTextEx(labelFont, a.title.c_str(), {a.rec.x + 36, a.rec.y - 2}, 20, 1,
               hovered ? gold : textLight);
    DrawTextEx(smallFont, a.desc.c_str(), {a.rec.x + 36, a.rec.y + 18}, 15, 1,
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
             {deckRect.x + 34, deckRect.y + deckRect.height / 2 - 10}, 20, 1,
             textMuted);
}

void GameScene::drawBoard() {
  DrawRectangleRounded(boardRect, 0.02f, 6, Color{18, 25, 28, 255});

  DrawRectangleGradientV((int)boardRect.x, (int)boardRect.y,
                         (int)boardRect.width, (int)boardRect.height,
                         Color{26, 34, 38, 255}, Color{14, 19, 21, 255});
  DrawRectangleRoundedLines(boardRect, 0.02f, 6, 2, Color{90, 85, 70, 140});

  if (gameManager->isAwaitingHeroStartPlacement()) {
    Hero *h = gameManager->getHeroAwaitingStartPlacement();
    DrawTextEx(smallFont,
               TextFormat("Player %d: choose %s's starting space -- click a "
                          "highlighted space",
                          h ? h->getOwnerPlayer() + 1 : 0,
                          h ? h->getName().c_str() : "your hero"),
               {boardRect.x + 12, boardRect.y + 10}, 18, 1, gold);
  } else if (gameManager->isAwaitingSidekickPlacement()) {
    Fighter *sk = gameManager->getSidekickAwaitingPlacement();
    DrawTextEx(
        smallFont,
        TextFormat(
            "Place %s (Player %d's sidekick) -- click a highlighted space",
            sk ? sk->getName().c_str() : "sidekick",
            sk ? sk->getOwnerPlayer() + 1 : 0),
        {boardRect.x + 12, boardRect.y + 10}, 18, 1, gold);
  } else if (gameManager->isWaitingForTile()) {
    DrawTextEx(smallFont, "Choose a highlighted space",
               {boardRect.x + 12, boardRect.y + 10}, 18, 1, gold);
  } else if (gameManager->isWaitingForFighter()) {
    DrawTextEx(smallFont, "Choose a highlighted fighter, or Skip",
               {boardRect.x + 12, boardRect.y + 10}, 18, 1, gold);
    if (fighterChoiceDeclineRect.width > 0) {
      bool hovered =
          CheckCollisionPointRec(GetMousePosition(), fighterChoiceDeclineRect);
      DrawRectangleRounded(fighterChoiceDeclineRect, 0.15f, 6,
                           hovered ? Color{70, 45, 60, 255}
                                   : Color{45, 30, 40, 255});
      DrawRectangleRoundedLines(fighterChoiceDeclineRect, 0.15f, 6, 2, gold);
      DrawTextEx(
          smallFont, "Skip",
          {fighterChoiceDeclineRect.x + 14, fighterChoiceDeclineRect.y + 8}, 18,
          1, textLight);
    }
  }
  if (moveModeActive) {
    DrawTextEx(smallFont,
               "Choose a highlighted tile to move to, or click a hand card to "
               "Boost (Esc to cancel)",
               {boardRect.x + 12, boardRect.y + 10}, 18, 1, gold);
  } else if (gameManager->canBoostMovement()) {

    DrawTextEx(
        smallFont,
        "No legal move right now -- click a hand card to Boost your movement",
        {boardRect.x + 12, boardRect.y + 10}, 18, 1, gold);
  } else if (!targetModeActive && !gameManager->isWaitingForTile() &&
             !gameManager->isAwaitingSidekickPlacement()) {
    Fighter *active = getActiveFighter();
    std::string label = active ? active->getName() : "";
    DrawTextEx(
        smallFont,
        TextFormat("Active fighter: %s -- click your own token to switch",
                   label.c_str()),
        {boardRect.x + 12, boardRect.y + 10}, 18, 1, textMuted);
  }
  if (targetModeActive) {
    const char *msg =
        pendingTargetIsAttack
            ? "Choose a highlighted enemy to attack (Esc to cancel)"
            : "Choose a highlighted fighter to target (Esc to cancel)";
    DrawTextEx(smallFont, msg, {boardRect.x + 12, boardRect.y + 10}, 18, 1,
               gold);
  } else if (attackModeActive) {
    const char *msg = attackRangeTargets.empty()
                          ? "No enemy in range -- move closer first"
                          : "Highlighted enemies are in range -- click an "
                            "Attack/Multipurpose card in your hand";
    DrawTextEx(smallFont, msg, {boardRect.x + 12, boardRect.y + 10}, 18, 1,
               gold);
  } else if (schemeModeActive) {
    DrawTextEx(smallFont, "Click a Scheme (event) card in your hand to play it",
               {boardRect.x + 12, boardRect.y + 10}, 18, 1, gold);
  }

  for (auto &e : edges) {
    auto itA = idToIndex.find(e.first);
    auto itB = idToIndex.find(e.second);
    if (itA == idToIndex.end() || itB == idToIndex.end())
      continue;
    Vector2 a = tiles[itA->second].pos;
    Vector2 b = tiles[itB->second].pos;
    a.x += boardRect.x;
    a.y += boardRect.y;
    b.x += boardRect.x;
    b.y += boardRect.y;
    DrawLineEx(a, b, 6, Color{20, 18, 15, 255});
  }

  std::vector<Tile *> genericPickOptions = gameManager->isWaitingForTile()
                                               ? gameManager->getValidTiles()
                                               : std::vector<Tile *>{};

  for (auto &t : tiles) {
    Vector2 c = {t.pos.x + boardRect.x, t.pos.y + boardRect.y};
    if (t.hasThirdZone) {

      DrawCircleSector(c, t.radius, -90, 30, 24, t.color);
      DrawCircleSector(c, t.radius, 30, 150, 24, t.color2);
      DrawCircleSector(c, t.radius, 150, 270, 24, t.color3);
      for (float ang : {-90.0f, 30.0f, 150.0f}) {
        float rad = ang * (3.14159265f / 180.0f);
        DrawLineEx(c, {c.x + t.radius * cosf(rad), c.y + t.radius * sinf(rad)},
                   2, Color{0, 0, 0, 70});
      }
    } else if (t.hasSecondZone) {

      DrawCircleSector(c, t.radius, 90, 270, 32, t.color);
      DrawCircleSector(c, t.radius, 270, 450, 32, t.color2);
      DrawLineEx({c.x, c.y - t.radius}, {c.x, c.y + t.radius}, 2,
                 Color{0, 0, 0, 70});
    } else {
      DrawCircleV(c, t.radius, t.color);
    }

    if (t.isPortal) {

      DrawCircleLines((int)c.x, (int)c.y, t.radius - 6, portalRingColor);
      DrawCircleLines((int)c.x, (int)c.y, t.radius - 8, portalRingColor);
    }

    bool isGenericPickTile =
        std::find_if(genericPickOptions.begin(), genericPickOptions.end(),
                     [&](Tile *opt) { return opt->getId() == t.id; }) !=
        genericPickOptions.end();
    if (isGenericPickTile) {
      Color ring =
          (t.id == hoveredGenericTile) ? gold : Color{120, 190, 220, 220};
      DrawCircleLines((int)c.x, (int)c.y, t.radius + 4, ring);
      DrawCircleLines((int)c.x, (int)c.y, t.radius + 5, ring);
    }

    bool isFighterChoiceTile =
        gameManager->isWaitingForFighter() && t.occupant && [&] {
          auto opts = gameManager->getValidFighters();
          return std::find(opts.begin(), opts.end(), t.occupant) != opts.end();
        }();
    if (isFighterChoiceTile) {
      Color ring =
          (t.id == hoveredFighterChoiceTile) ? gold : Color{220, 90, 200, 220};
      DrawCircleLines((int)c.x, (int)c.y, t.radius + 4, ring);
      DrawCircleLines((int)c.x, (int)c.y, t.radius + 5, ring);
    }

    bool isLegalMoveTile =
        moveModeActive &&
        std::find(legalMoveTiles.begin(), legalMoveTiles.end(), t.id) !=
            legalMoveTiles.end();
    if (isLegalMoveTile) {
      Color ring = (t.id == hoveredMoveTile) ? gold : Color{120, 200, 120, 220};
      DrawCircleLines((int)c.x, (int)c.y, t.radius + 4, ring);
      DrawCircleLines((int)c.x, (int)c.y, t.radius + 5, ring);
    }

    bool isLegalTargetTile = targetModeActive && [&] {
      Fighter *fighter = gameManager->getMap().getFighterAt(t.id);
      return fighter && std::find(legalTargets.begin(), legalTargets.end(),
                                  fighter) != legalTargets.end();
    }();
    if (isLegalTargetTile) {
      Color ring = (t.id == hoveredTargetTile) ? gold : Color{200, 90, 90, 220};
      DrawCircleLines((int)c.x, (int)c.y, t.radius + 4, ring);
      DrawCircleLines((int)c.x, (int)c.y, t.radius + 5, ring);
    }

    if (attackModeActive && !targetModeActive) {
      bool inRange = std::find(attackRangeTiles.begin(), attackRangeTiles.end(),
                               t.id) != attackRangeTiles.end();
      if (inRange) {
        DrawCircleLines((int)c.x, (int)c.y, t.radius + 4,
                        Color{230, 150, 60, 150});
        DrawCircleLines((int)c.x, (int)c.y, t.radius + 5,
                        Color{230, 150, 60, 150});

        bool isTarget =
            t.occupant &&
            std::find(attackRangeTargets.begin(), attackRangeTargets.end(),
                      t.occupant) != attackRangeTargets.end();
        if (isTarget) {
          DrawCircleLines((int)c.x, (int)c.y, t.radius + 7,
                          Color{220, 60, 40, 230});
          DrawCircleLines((int)c.x, (int)c.y, t.radius + 8,
                          Color{220, 60, 40, 230});
        }
      }
    }

    DrawCircleLines((int)c.x, (int)c.y, t.radius, Color{0, 0, 0, 90});

    if (t.occupant) {

      Color ownerTint = (t.occupant->getOwnerPlayer() % 2 == 0)
                            ? SKYBLUE
                            : Color{220, 120, 120, 255};
      bool isHero = t.occupant->getFighterType() == TypeOfFighter::hero;

      float tokenRadius = isHero ? 16.0f : 11.0f;
      DrawCircleV(c, tokenRadius, ownerTint);
      DrawCircleLines((int)c.x, (int)c.y, (int)tokenRadius,
                      isHero ? gold : DARKGRAY);
      if (isHero) {
        DrawCircleLines((int)c.x, (int)c.y, (int)tokenRadius + 2, gold);
      }
      if (t.occupant == getActiveFighter()) {

        DrawCircleLines((int)c.x, (int)c.y, (int)tokenRadius + 5, WHITE);
        DrawCircleLines((int)c.x, (int)c.y, (int)tokenRadius + 6, WHITE);
      }

      std::string label = t.occupant->getName().empty()
                              ? "?"
                              : t.occupant->getName().substr(0, 1);
      Vector2 labelSize = MeasureTextEx(labelFont, label.c_str(), 18, 1);
      DrawTextEx(labelFont, label.c_str(),
                 {c.x - labelSize.x / 2.0f, c.y - labelSize.y / 2.0f}, 18, 1,
                 BLACK);
    }
  }
}

void GameScene::drawHand() {
  for (size_t i = 0; i < hand.size(); i++) {
    Rectangle &r = handRects[i];
    HandCardView &c = hand[i];
    bool hovered = ((int)i == selectedCard);

    Rectangle drawRect = r;
    if (hovered)
      drawRect.y -= 10.0f;

    Rectangle shadowRect = drawRect;
    shadowRect.x += 4.0f;
    shadowRect.y += (hovered ? 10.0f : 6.0f);
    if (hovered) {
      DrawRectangleRounded(shadowRect, 0.06f, 6,
                           Color{0, 0, 0, 110 });

    } else {
     DrawRectangleRounded(shadowRect, 0.06f, 6,
                           Color{0, 0, 0, 80 });
    }

    DrawRectangleRounded(drawRect, 0.06f, 6, Color{30, 27, 22, 255});

    if (c.art) {
      Rectangle src{0, 0, (float)c.art->width, (float)c.art->height};
      DrawTexturePro(*c.art, src, drawRect, {0, 0}, 0.0f, WHITE);
    }

    DrawRectangleRoundedLines(drawRect, 0.06f, 6, hovered ? 5.0f : 4.0f,
                              hovered ? gold : Color{110, 100, 80, 255});
  }
}

void GameScene::drawBottomBar() {
  DrawRectangleRounded({20, sh - 90, 90, 60}, 0.2f, 6, panelBg);
  DrawTextEx(titleFont, TextFormat("%d", cardsInDeck), {55, sh - 74}, 30, 1,
             textLight);

  DrawRectangleRounded({120, sh - 90, 90, 60}, 0.2f, 6, panelBg);
  DrawCircleV({160, sh - 60}, 14, Color{150, 20, 20, 255});
  DrawTextEx(titleFont, TextFormat("%d", tokens), {178, sh - 74}, 30, 1,
             textLight);

  DrawTextEx(labelFont, "UNMATCHED", {20, sh - 26}, 20, 1, textMuted);
  DrawTextEx(smallFont, mapName.c_str(), {160, sh - 26}, 20, 1, gold);

  DrawRectangleRounded(endTurnRect, 0.15f, 6, Color{25, 55, 90, 255});
  DrawRectangleRoundedLines(endTurnRect, 0.15f, 6, 2, SKYBLUE);
  DrawTextEx(labelFont, "END TURN", {endTurnRect.x + 22, endTurnRect.y + 11},
             24, 1, WHITE);

  if (gameManager->isManeuverActive()) {
    DrawRectangleRounded(finishMovingRect, 0.15f, 6, Color{55, 45, 20, 255});
    DrawRectangleRoundedLines(finishMovingRect, 0.15f, 6, 2, gold);
    DrawTextEx(labelFont, "FINISH MOVING",
               {finishMovingRect.x + 8, finishMovingRect.y + 11}, 18, 1, WHITE);
  } else if (gameManager->getStayTileOption() != nullptr) {
    DrawRectangleRounded(stayPutRect, 0.15f, 6, Color{55, 45, 20, 255});
    DrawRectangleRoundedLines(stayPutRect, 0.15f, 6, 2, gold);
    DrawTextEx(labelFont, "DON'T MOVE", {stayPutRect.x + 8, stayPutRect.y + 11},
               18, 1, WHITE);
  }
}

void GameScene::drawEventLog() {
  DrawRectangleRounded(eventLogRect, 0.06f, 8, panelBg);
  DrawRectangleRoundedLines(eventLogRect, 0.06f, 8, 2,
                            Color{110, 100, 80, 255});

  DrawTextEx(labelFont, "LOG", {eventLogRect.x + 14, eventLogRect.y + 10}, 18,
             1, gold);

  float x = eventLogRect.x + 14;
  float y = eventLogRect.y + 40;
  if (eventLog.empty()) {
    DrawTextEx(smallFont, "Nothing has happened yet.", {x, y}, 15, 1,
               textMuted);
    return;
  }
  for (auto &line : eventLog) {
    DrawTextEx(smallFont, line.c_str(), {x, y}, 15, 1, textMuted);
    y += 24;
  }
}

void GameScene::drawResultScreen() {
  DrawRectangle(0, 0, (int)sw, (int)sh, Color{0, 0, 0, 190});

  Rectangle panel = {sw / 2 - 220, sh / 2 - 130, 440, 300};
  DrawRectangleRounded(panel, 0.08f, 8, panelBg);
  DrawRectangleRoundedLines(panel, 0.08f, 8, 3, gold);

  const char *title = "VICTORY";
  int titleWidth = MeasureTextEx(titleFont, title, 40, 1).x;
  DrawTextEx(titleFont, title,
             {panel.x + panel.width / 2 - titleWidth / 2.0f, panel.y + 34}, 40,
             1, gold);

  std::string subtitle = winnerName.empty() ? "The match has ended."
                                            : (winnerName + " wins the match!");
  int subtitleWidth = MeasureTextEx(labelFont, subtitle.c_str(), 22, 1).x;
  DrawTextEx(labelFont, subtitle.c_str(),
             {panel.x + panel.width / 2 - subtitleWidth / 2.0f, panel.y + 100},
             22, 1, textLight);

  bool hovered = CheckCollisionPointRec(GetMousePosition(), resultMenuRect);
  DrawRectangleRounded(resultMenuRect, 0.15f, 6, Color{25, 55, 90, 255});
  DrawRectangleRoundedLines(resultMenuRect, 0.15f, 6, 2,
                            hovered ? gold : SKYBLUE);
  const char *buttonLabel = "RETURN TO MENU";
  float buttonWidth = MeasureTextEx(labelFont, buttonLabel, 20, 1).x;
  DrawTextEx(labelFont, buttonLabel,
             {resultMenuRect.x + resultMenuRect.width / 2 - buttonWidth / 2,
              resultMenuRect.y + 14},
             20, 1, WHITE);
}

void GameScene::drawDefensePrompt() {
  DrawRectangle(0, 0, (int)sw, (int)sh, Color{0, 0, 0, 190});

  DrawRectangleRounded(defensePanelRect, 0.06f, 8, panelBg);
  DrawRectangleRoundedLines(defensePanelRect, 0.06f, 8, 3, gold);

  Fighter *attacker = gameManager->getCombatAttacker();
  Fighter *defender = gameManager->getCombatDefender();
  Card *attackCard = gameManager->getCombatAttackerCard();

  const char *title = "DEFEND!";
  int titleWidth = MeasureTextEx(titleFont, title, 30, 1).x;
  DrawTextEx(
      titleFont, title,
      {defensePanelRect.x + defensePanelRect.width / 2 - titleWidth / 2.0f,
       defensePanelRect.y + 16},
      30, 1, gold);

  std::string subtitle =
      (attacker ? attacker->getName() : "Attacker") + " hits " +
      (defender ? defender->getName() : "you") + " with " +
      (attackCard ? attackCard->getName() : "an attack") +
      (attackCard ? TextFormat(" (%d)", attackCard->getAttackStat()) : "");
  DrawTextEx(smallFont, subtitle.c_str(),
             {defensePanelRect.x + 20, defensePanelRect.y + 56}, 15, 1,
             textMuted);

  Vector2 mouse = GetMousePosition();

  if (!defenseHandRevealed) {

    bool revealHovered = CheckCollisionPointRec(mouse, revealHandRect);
    DrawRectangleRounded(revealHandRect, 0.1f, 6, Color{22, 20, 18, 235});
    BeginScissorMode((int)revealHandRect.x, (int)revealHandRect.y,
                     (int)revealHandRect.width, (int)revealHandRect.height);
    for (float x = -revealHandRect.height; x < revealHandRect.width;
         x += 14.0f) {
      Vector2 p1 = {revealHandRect.x + x, revealHandRect.y};
      Vector2 p2 = {revealHandRect.x + x + revealHandRect.height,
                    revealHandRect.y + revealHandRect.height};
      DrawLineEx(p1, p2, 3.0f, Color{60, 54, 44, 255});
    }
    EndScissorMode();
    DrawRectangleRoundedLines(revealHandRect, 0.1f, 6, 2,
                              revealHovered ? gold : Color{110, 100, 80, 255});

    const char *lockLabel = "Defender's eyes only";
    int lockW = MeasureTextEx(labelFont, lockLabel, 17, 1).x;
    DrawTextEx(labelFont, lockLabel,
               {revealHandRect.x + revealHandRect.width / 2 - lockW / 2.0f,
                revealHandRect.y + revealHandRect.height / 2 - 26},
               17, 1, gold);
    const char *tapLabel = "Tap to reveal your hand";
    int tapW = MeasureTextEx(smallFont, tapLabel, 14, 1).x;
    DrawTextEx(smallFont, tapLabel,
               {revealHandRect.x + revealHandRect.width / 2 - tapW / 2.0f,
                revealHandRect.y + revealHandRect.height / 2 + 2},
               14, 1, textMuted);
    return;
  }

  for (size_t i = 0;
       i < defenseOptionRects.size() && i < defenseCardOptions.size(); i++) {
    Rectangle &r = defenseOptionRects[i];
    Card *c = defenseCardOptions[i];
    bool hovered = CheckCollisionPointRec(mouse, r);

    DrawRectangleRounded(r, 0.1f, 6, Color{30, 27, 22, 255});
    DrawRectangleRoundedLines(r, 0.1f, 6, 2,
                              hovered ? gold : Color{110, 100, 80, 255});
    DrawTextEx(labelFont, c->getName().c_str(), {r.x + 12, r.y + 8}, 16, 1,
               textLight);
    DrawTextEx(smallFont, TextFormat("DEF %d", c->getDefStat()),
               {r.x + r.width - 70, r.y + 12}, 14, 1, SKYBLUE);
  }

  bool noDefHovered = CheckCollisionPointRec(mouse, noDefenseRect);
  DrawRectangleRounded(noDefenseRect, 0.1f, 6, Color{45, 30, 30, 255});
  DrawRectangleRoundedLines(noDefenseRect, 0.1f, 6, 2,
                            noDefHovered ? gold : Color{140, 90, 90, 255});
  DrawTextEx(labelFont, "No Defense (bare-handed, DEF 0)",
             {noDefenseRect.x + 12, noDefenseRect.y + 14}, 15, 1, textLight);
}

void GameScene::drawCardChoicePrompt() {
  DrawRectangle(0, 0, (int)sw, (int)sh, Color{0, 0, 0, 190});

  DrawRectangleRounded(cardChoicePanelRect, 0.06f, 8, panelBg);
  DrawRectangleRoundedLines(cardChoicePanelRect, 0.06f, 8, 3, gold);

  const char *title = "CHOOSE A CARD";
  int titleWidth = MeasureTextEx(titleFont, title, 26, 1).x;
  DrawTextEx(titleFont, title,
             {cardChoicePanelRect.x + cardChoicePanelRect.width / 2 -
                  titleWidth / 2.0f,
              cardChoicePanelRect.y + 16},
             26, 1, gold);
  DrawTextEx(smallFont, "Pick one, or stop if you're done",
             {cardChoicePanelRect.x + 20, cardChoicePanelRect.y + 56}, 14, 1,
             textMuted);

  std::vector<Card *> options = gameManager->isWaitingForCard()
                                    ? gameManager->getValidCards()
                                    : std::vector<Card *>{};
  std::vector<Card *> realOptions;
  for (Card *c : options) {
    if (c)
      realOptions.push_back(c);
  }

  Vector2 mouse = GetMousePosition();
  for (size_t i = 0; i < cardChoiceOptionRects.size() && i < realOptions.size();
       i++) {
    Rectangle &r = cardChoiceOptionRects[i];
    Card *c = realOptions[i];
    bool hovered = CheckCollisionPointRec(mouse, r);

    DrawRectangleRounded(r, 0.1f, 6, Color{30, 27, 22, 255});
    DrawRectangleRoundedLines(r, 0.1f, 6, 2,
                              hovered ? gold : Color{110, 100, 80, 255});
    DrawTextEx(labelFont, c->getName().c_str(), {r.x + 12, r.y + 8}, 16, 1,
               textLight);
    DrawTextEx(smallFont, TextFormat("Boost %d", c->getBoost()),
               {r.x + r.width - 80, r.y + 12}, 14, 1, SKYBLUE);
  }

  bool hasDecline =
      std::find(options.begin(), options.end(), nullptr) != options.end();
  if (hasDecline) {
    bool declineHovered = CheckCollisionPointRec(mouse, cardChoiceDeclineRect);
    DrawRectangleRounded(cardChoiceDeclineRect, 0.1f, 6,
                         Color{45, 30, 30, 255});
    DrawRectangleRoundedLines(cardChoiceDeclineRect, 0.1f, 6, 2,
                              declineHovered ? gold : Color{140, 90, 90, 255});
    DrawTextEx(labelFont, "Stop (discard no more)",
               {cardChoiceDeclineRect.x + 12, cardChoiceDeclineRect.y + 14}, 15,
               1, textLight);
  }
}

void GameScene::drawHandRevealPrompt() {
  DrawRectangle(0, 0, (int)sw, (int)sh, Color{0, 0, 0, 190});

  DrawRectangleRounded(handRevealPanelRect, 0.06f, 8, panelBg);
  DrawRectangleRoundedLines(handRevealPanelRect, 0.06f, 8, 3, gold);

  std::string title = handRevealOwnerName.empty()
                          ? "HAND REVEALED"
                          : handRevealOwnerName + "'s HAND";
  int titleWidth = MeasureTextEx(titleFont, title.c_str(), 24, 1).x;
  DrawTextEx(titleFont, title.c_str(),
             {handRevealPanelRect.x + handRevealPanelRect.width / 2 -
                  titleWidth / 2.0f,
              handRevealPanelRect.y + 16},
             24, 1, gold);
  DrawTextEx(smallFont, "Revealed by Study Methods",
             {handRevealPanelRect.x + 20, handRevealPanelRect.y + 56}, 14, 1,
             textMuted);

  if (handRevealCards.empty()) {
    DrawTextEx(smallFont, "(empty hand)",
               {handRevealPanelRect.x + 20, handRevealPanelRect.y + 90}, 14, 1,
               textMuted);
  }

  for (size_t i = 0;
       i < handRevealCardRects.size() && i < handRevealCards.size(); i++) {
    Rectangle &r = handRevealCardRects[i];
    Card *c = handRevealCards[i];
    if (!c)
      continue;

    DrawRectangleRounded(r, 0.1f, 6, Color{30, 27, 22, 255});
    DrawRectangleRoundedLines(r, 0.1f, 6, 2, Color{110, 100, 80, 255});
    DrawTextEx(labelFont, c->getName().c_str(), {r.x + 12, r.y + 8}, 16, 1,
               textLight);

    bool hasStat = c->getCardType() != TypeOfCard::event;
    if (hasStat) {
      int statValue = c->getCardType() == TypeOfCard::def ? c->getDefStat()
                                                          : c->getAttackStat();
      DrawTextEx(smallFont, TextFormat("%d", statValue),
                 {r.x + r.width - 70, r.y + 12}, 14, 1, SKYBLUE);
    }
    DrawTextEx(smallFont, TextFormat("Boost %d", c->getBoost()),
               {r.x + r.width - 40, r.y + 12}, 14, 1, gold);
  }

  Vector2 mouse = GetMousePosition();
  bool closeHovered = CheckCollisionPointRec(mouse, handRevealCloseRect);
  DrawRectangleRounded(handRevealCloseRect, 0.1f, 6, Color{45, 30, 30, 255});
  DrawRectangleRoundedLines(handRevealCloseRect, 0.1f, 6, 2,
                            closeHovered ? gold : Color{140, 90, 90, 255});
  const char *closeLabel = "Close";
  int closeW = MeasureTextEx(labelFont, closeLabel, 15, 1).x;
  DrawTextEx(
      labelFont, closeLabel,
      {handRevealCloseRect.x + handRevealCloseRect.width / 2 - closeW / 2.0f,
       handRevealCloseRect.y + 14},
      15, 1, textLight);
}

void GameScene::handleHandRevealMouse() {
  if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    return;
  Vector2 mouse = GetMousePosition();
  if (CheckCollisionPointRec(mouse, handRevealCloseRect)) {
    handRevealActive = false;
  }
}

void GameScene::handleCardChoiceMouse() {
  if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    return;
  Vector2 mouse = GetMousePosition();

  std::vector<Card *> options = gameManager->getValidCards();
  std::vector<Card *> realOptions;
  for (Card *c : options) {
    if (c)
      realOptions.push_back(c);
  }

  for (size_t i = 0; i < cardChoiceOptionRects.size() && i < realOptions.size();
       i++) {
    if (CheckCollisionPointRec(mouse, cardChoiceOptionRects[i])) {
      gameManager->submitCard(realOptions[i]);
      refreshFromGameManager();
      return;
    }
  }

  bool hasDecline =
      std::find(options.begin(), options.end(), nullptr) != options.end();
  if (hasDecline && CheckCollisionPointRec(mouse, cardChoiceDeclineRect)) {
    gameManager->submitCard(nullptr);
    refreshFromGameManager();
  }
}

bool GameScene::isCardChoicePromptActive() const {
  return gameManager->isWaitingForCard();
}

void GameScene::drawDiscardPrompt() {

  int handCount = (int)hand.size();
  std::string banner = TextFormat(
      "Hand limit exceeded (%d/7) -- click a card to discard", handCount);
  int textW = MeasureText(banner.c_str(), 18);
  DrawRectangle((int)(sw / 2 - textW / 2 - 14),
                (int)(handRects.empty() ? sh - 334 : handRects[0].y - 34),
                textW + 28, 26, Color{20, 15, 15, 220});
  DrawTextEx(labelFont, banner.c_str(),
             {sw / 2 - textW / 2.0f,
              handRects.empty() ? sh - 330.0f : handRects[0].y - 30},
             18, 1, RED);

  Vector2 mouse = GetMousePosition();
  for (auto &r : handRects) {
    if (CheckCollisionPointRec(mouse, r)) {
      DrawRectangleRoundedLines(r, 0.08f, 6, 3, RED);
    }
  }
}

void GameScene::handleDiscardMouse() {
  if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    return;
  Vector2 mouse = GetMousePosition();
  for (size_t i = 0; i < handRects.size() && i < hand.size(); i++) {
    if (CheckCollisionPointRec(mouse, handRects[i])) {
      gameManager->discardExcessCard(hand[i].card);
      refreshFromGameManager();

      return;
    }
  }
}

void GameScene::tryBoostMovement(int index) {
  if (index < 0 || index >= (int)hand.size())
    return;
  Card *card = hand[index].card;

  if (gameManager->boostMovement(card, getActiveFighter())) {
    pushLog(eventLog, "Boosted movement with " + hand[index].name + ".");
    refreshFromGameManager();

    beginMovePicker();
  } else {
    pushLog(eventLog, "Can't Boost movement with that card right now.");
  }
}

void GameScene::playHandCard(int index) {
  if (index < 0 || index >= (int)hand.size())
    return;
  if (combatDefensePending || gameManager->isCombatActive() ||
      isDiscardPromptActive())
    return;
  cancelMovePicker();

  Card *card = hand[index].card;
  TypeOfCard type = card->getCardType();

  Fighter *activeFighter = getActiveFighter();
  if (!gameManager->canPerform(card, activeFighter)) {

    pushLog(eventLog, "This card can't be played by " +
                          (activeFighter ? activeFighter->getName()
                                         : std::string("that fighter")) +
                          ".");
    return;
  }

  if (type == TypeOfCard::attack || type == TypeOfCard::multipurpose) {

    beginAttackPicker(card);
    return;
  }

  if (gameManager->cardNeedsTarget(card)) {

    beginTargetPicker(card);
    return;
  }

  schemeModeActive = false;

  Fighter *self = getActiveFighter();
  Hero *enemy = getDefaultEnemyHero();
  gameManager->playCard(card, self, enemy, enemy);
  refreshFromGameManager();
}

void GameScene::handleGenericTilePick() {
  Vector2 mouse = GetMousePosition();
  hoveredGenericTile = -1;
  if (!CheckCollisionPointRec(mouse, boardRect))
    return;

  std::vector<Tile *> options = gameManager->getValidTiles();
  for (auto &t : tiles) {
    Vector2 c = {t.pos.x + boardRect.x, t.pos.y + boardRect.y};
    if (!CheckCollisionPointCircle(mouse, c, t.radius))
      continue;

    auto it = std::find_if(options.begin(), options.end(),
                           [&](Tile *opt) { return opt->getId() == t.id; });
    if (it == options.end())
      break;

    hoveredGenericTile = t.id;
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      gameManager->submitTile(*it);

      refreshFromGameManager();
    }
    break;
  }
}

void GameScene::handleGenericFighterPick() {
  Vector2 mouse = GetMousePosition();
  hoveredFighterChoiceTile = -1;

  std::vector<Fighter *> options = gameManager->getValidFighters();
  bool hasDecline =
      std::find(options.begin(), options.end(), nullptr) != options.end();

  fighterChoiceDeclineRect =
      hasDecline ? Rectangle{boardRect.x + 12,
                             boardRect.y + boardRect.height - 46, 160, 34}
                 : Rectangle{0, 0, 0, 0};
  if (hasDecline && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
      CheckCollisionPointRec(mouse, fighterChoiceDeclineRect)) {
    gameManager->submitFighter(nullptr);
    refreshFromGameManager();
    return;
  }

  if (!CheckCollisionPointRec(mouse, boardRect))
    return;

  for (auto &t : tiles) {
    Vector2 c = {t.pos.x + boardRect.x, t.pos.y + boardRect.y};
    if (!CheckCollisionPointCircle(mouse, c, t.radius))
      continue;

    Fighter *fighter = gameManager->getMap().getFighterAt(t.id);
    bool isLegal = fighter && std::find(options.begin(), options.end(),
                                        fighter) != options.end();
    if (!isLegal)
      break;

    hoveredFighterChoiceTile = t.id;
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      gameManager->submitFighter(fighter);
      refreshFromGameManager();
    }
    break;
  }
}

void GameScene::handleMouse() {

  if (gameManager->isWaitingForTile()) {
    handleGenericTilePick();
    return;
  }
  if (gameManager->isWaitingForFighter()) {
    handleGenericFighterPick();
    return;
  }

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
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (gameManager->canBoostMovement()) {

          tryBoostMovement(selectedCard);
        } else {
          playHandCard(selectedCard);
        }
      }
      break;
    }
  }

  if (CheckCollisionPointRec(mouse, endTurnRect) &&
      IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    activateAction((int)actionButtons.size() - 1);
  }

  if (gameManager->isManeuverActive() &&
      CheckCollisionPointRec(mouse, finishMovingRect) &&
      IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    gameManager->finishManeuver();
    cancelMovePicker();
    pushLog(eventLog, "Finished moving.");
    refreshFromGameManager();
  }

  if (Tile *stayTile = gameManager->getStayTileOption()) {
    if (CheckCollisionPointRec(mouse, stayPutRect) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      gameManager->submitTile(stayTile);
      pushLog(eventLog, "Chose not to move.");
      refreshFromGameManager();
    }
  }

  hoveredMoveTile = -1;
  if (moveModeActive && CheckCollisionPointRec(mouse, boardRect)) {
    for (auto &t : tiles) {
      Vector2 c = {t.pos.x + boardRect.x, t.pos.y + boardRect.y};
      if (CheckCollisionPointCircle(mouse, c, t.radius)) {
        bool isLegal = std::find(legalMoveTiles.begin(), legalMoveTiles.end(),
                                 t.id) != legalMoveTiles.end();
        if (isLegal) {
          hoveredMoveTile = t.id;
          if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            tryMoveToTile(t.id);
          }
        } else if (t.occupant && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {

          trySelectActiveFighter(t.occupant);
        }
        break;
      }
    }
  }

  if (moveModeActive && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
    cancelMovePicker();
  }

  hoveredTargetTile = -1;
  if (targetModeActive && CheckCollisionPointRec(mouse, boardRect)) {
    for (auto &t : tiles) {
      Vector2 c = {t.pos.x + boardRect.x, t.pos.y + boardRect.y};
      if (CheckCollisionPointCircle(mouse, c, t.radius)) {
        Fighter *fighter = gameManager->getMap().getFighterAt(t.id);
        bool isLegal =
            fighter && std::find(legalTargets.begin(), legalTargets.end(),
                                 fighter) != legalTargets.end();
        if (isLegal) {
          hoveredTargetTile = t.id;
          if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            tryTargetFighter(fighter);
          }
        }
        break;
      }
    }
  }

  if (targetModeActive && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
    cancelTargetPicker();
  }

  if (!moveModeActive && !targetModeActive &&
      IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
      CheckCollisionPointRec(mouse, boardRect)) {
    for (auto &t : tiles) {
      Vector2 c = {t.pos.x + boardRect.x, t.pos.y + boardRect.y};
      if (CheckCollisionPointCircle(mouse, c, t.radius)) {
        if (t.occupant)
          trySelectActiveFighter(t.occupant);
        break;
      }
    }
  }
}

void GameScene::handleDefenseMouse() {
  if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    return;
  Vector2 mouse = GetMousePosition();

  if (!defenseHandRevealed) {
    if (CheckCollisionPointRec(mouse, revealHandRect)) {
      defenseHandRevealed = true;
    }
    return;
  }

  for (size_t i = 0;
       i < defenseOptionRects.size() && i < defenseCardOptions.size(); i++) {
    if (CheckCollisionPointRec(mouse, defenseOptionRects[i])) {
      Card *chosen = defenseCardOptions[i];
      if (chosen->needsPrediction()) {
        beginPredictionPrompt(chosen);
      } else {
        resolveDefense(chosen);
      }
      return;
    }
  }
  if (CheckCollisionPointRec(mouse, noDefenseRect)) {
    resolveDefense(nullptr);
  }
}

void GameScene::handleKeyboard() {
  if (gameManager->isWaitingForTile())
    return;

  if (moveModeActive && IsKeyPressed(KEY_ESCAPE)) {
    cancelMovePicker();
  }
  if (targetModeActive && IsKeyPressed(KEY_ESCAPE)) {
    cancelTargetPicker();
  }
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
  if (IsKeyPressed(KEY_ENTER) && selectedCard >= 0) {
    if (gameManager->canBoostMovement()) {
      tryBoostMovement(selectedCard);
    } else {
      playHandCard(selectedCard);
    }
  }
}

Fighter *GameScene::getActiveFighter() const {
  Hero *hero = gameManager->getCurrentHero();
  if (!hero)
    return nullptr;
  if (activeFighter && activeFighter->isAlive() &&
      activeFighter->getOwnerPlayer() == hero->getOwnerPlayer()) {
    return activeFighter;
  }
  return hero;
}

void GameScene::trySelectActiveFighter(Fighter *fighter) {
  if (!fighter || !fighter->isAlive())
    return;

  if (targetModeActive || gameManager->isWaitingForTile() ||
      combatDefensePending || isDiscardPromptActive()) {
    return;
  }
  Hero *hero = gameManager->getCurrentHero();
  if (!hero || fighter->getOwnerPlayer() != hero->getOwnerPlayer())
    return;

  if (activeFighter != fighter) {
    activeFighter = fighter;
    pushLog(eventLog, fighter->getName() + " is now the active fighter.");
  }

  beginMovePicker();

  if (attackModeActive)
    updateAttackRangePreview();
}

Hero *GameScene::getDefaultEnemyHero() const {
  Hero *hero = gameManager->getCurrentHero();
  if (!hero)
    return nullptr;
  for (int i = 0; i < gameManager->getPlayerCount(); i++) {
    Hero *candidate = gameManager->getHero(i);
    if (candidate && candidate->getOwnerPlayer() != hero->getOwnerPlayer()) {
      return candidate;
    }
  }
  return nullptr;
}

void GameScene::beginMovePicker() {
  legalMoveTiles.clear();
  hoveredMoveTile = -1;
  moveModeActive = false;

  Fighter *mover = getActiveFighter();
  if (!mover)
    return;

  int fromTileId = mover->getTileId();
  int moves = gameManager->getMovesRemaining(mover);
  if (moves <= 0)
    return;

  Map &board = gameManager->getMap();

  for (Tile *t : board.getReachableTiles(fromTileId, moves, mover)) {
    legalMoveTiles.push_back(t->getId());
  }
  moveModeActive = !legalMoveTiles.empty();
}

void GameScene::cancelMovePicker() {
  moveModeActive = false;
  legalMoveTiles.clear();
  hoveredMoveTile = -1;
}

void GameScene::tryMoveToTile(int tileId) {
  Fighter *mover = getActiveFighter();
  if (!mover) {
    cancelMovePicker();
    return;
  }

  gameManager->moveFighter(mover, tileId);
  cancelMovePicker();
  refreshFromGameManager();
}

void GameScene::beginTargetPicker(Card *card) {
  cancelTargetPicker();
  if (!card)
    return;

  auto targets = gameManager->getValidTargetsForCard(card);
  if (targets.empty())
    return;

  pendingTargetCard = card;
  legalTargets = std::move(targets);
  targetModeActive = true;
}

void GameScene::updateAttackRangePreview() {
  attackRangeTiles.clear();
  attackRangeTargets.clear();

  Fighter *self = getActiveFighter();
  if (!self)
    return;

  Map &board = gameManager->getMap();
  int fromTile = self->getTileId();
  Tile *fromTilePtr = board.getTile(fromTile);

  for (auto &t : tiles) {
    bool inRange;
    if (self->getTypeOfAttack() == TypeOfAttack::melee) {
      inRange = board.distanceBetween(fromTile, t.id) == 1;
    } else {

      bool adjacent = board.distanceBetween(fromTile, t.id) == 1;
      bool sameZone = false;
      if (!adjacent && fromTilePtr) {
        Tile *toTilePtr = board.getTile(t.id);
        if (toTilePtr) {
          for (const auto &zone : fromTilePtr->getZones()) {
            if (toTilePtr->getZones().count(zone)) {
              sameZone = true;
              break;
            }
          }
        }
      }
      inRange = adjacent || sameZone;
    }
    if (!inRange)
      continue;

    attackRangeTiles.push_back(t.id);
    if (t.occupant && t.occupant->isAlive() &&
        t.occupant->getOwnerPlayer() != self->getOwnerPlayer()) {
      attackRangeTargets.push_back(t.occupant);
    }
  }
}

void GameScene::clearAttackRangePreview() {
  attackRangeTiles.clear();
  attackRangeTargets.clear();
}

void GameScene::beginAttackPicker(Card *card) {
  cancelTargetPicker();
  if (!card)
    return;

  Fighter *self = getActiveFighter();
  if (!self)
    return;

  updateAttackRangePreview();

  if (attackRangeTargets.empty()) {
    pushLog(eventLog, "No legal attack target in range.");
    return;
  }

  pendingTargetCard = card;
  legalTargets = attackRangeTargets;
  targetModeActive = true;
  pendingTargetIsAttack = true;
}

void GameScene::cancelTargetPicker() {
  targetModeActive = false;
  pendingTargetCard = nullptr;
  legalTargets.clear();
  hoveredTargetTile = -1;
  pendingTargetIsAttack = false;
}

void GameScene::tryTargetFighter(Fighter *fighter) {
  if (!targetModeActive || !pendingTargetCard || !fighter)
    return;
  if (std::find(legalTargets.begin(), legalTargets.end(), fighter) ==
      legalTargets.end())
    return;

  Card *card = pendingTargetCard;
  bool isAttack = pendingTargetIsAttack;
  cancelTargetPicker();

  if (isAttack) {

    if (!gameManager->startCombat(card, getActiveFighter(), fighter)) {
      pushLog(eventLog, "That attack isn't legal anymore.");
      refreshFromGameManager();
      return;
    }
    attackModeActive = false;
    clearAttackRangePreview();
    beginDefensePrompt();
    refreshFromGameManager();
    return;
  }

  Fighter *self = getActiveFighter();

  Fighter *enemy = gameManager->getHero(fighter->getOwnerPlayer());

  gameManager->playCard(card, self, fighter, enemy);
  refreshFromGameManager();
}

void GameScene::beginDefensePrompt() {
  defenseCardOptions.clear();
  Hero *defender = gameManager->getCombatDefendingHero();
  if (defender && defender->getDeck()) {
    for (Card *c : defender->getDeck()->getHand()) {
      if (c->getCardType() == TypeOfCard::def ||
          c->getCardType() == TypeOfCard::multipurpose) {
        defenseCardOptions.push_back(c);
      }
    }
  }

  combatDefensePending = gameManager->isCombatActive();
  defenseHandRevealed = false;
  predictionPromptActive = false;
  pendingPredictionCard = nullptr;
}

void GameScene::resolveDefense(Card *defenseCard, int predictedValue) {
  if (!combatDefensePending)
    return;

  gameManager->resolveCombat(defenseCard, predictedValue);
  combatDefensePending = false;
  defenseCardOptions.clear();
  refreshFromGameManager();
}

void GameScene::beginPredictionPrompt(Card *defenseCard) {
  pendingPredictionCard = defenseCard;
  predictionPromptActive = true;
}

void GameScene::resolvePrediction(int guessedValue) {
  if (!predictionPromptActive || !pendingPredictionCard)
    return;
  Card *card = pendingPredictionCard;
  predictionPromptActive = false;
  pendingPredictionCard = nullptr;
  resolveDefense(card, guessedValue);
}

void GameScene::drawPredictionPrompt() {
  DrawRectangle(0, 0, (int)sw, (int)sh, Color{0, 0, 0, 190});

  DrawRectangleRounded(predictionPanelRect, 0.06f, 8, panelBg);
  DrawRectangleRoundedLines(predictionPanelRect, 0.06f, 8, 3, gold);

  const char *title = "PREDICT THE ATTACK";
  int titleWidth = MeasureTextEx(titleFont, title, 24, 1).x;
  DrawTextEx(titleFont, title,
             {predictionPanelRect.x + predictionPanelRect.width / 2 -
                  titleWidth / 2.0f,
              predictionPanelRect.y + 16},
             24, 1, gold);

  std::string subtitle = pendingPredictionCard
                             ? pendingPredictionCard->getName() +
                                   ": guess the opponent's printed attack value"
                             : "Guess the opponent's printed attack value";
  DrawTextEx(smallFont, subtitle.c_str(),
             {predictionPanelRect.x + 20, predictionPanelRect.y + 56}, 14, 1,
             textMuted);

  Vector2 mouse = GetMousePosition();
  for (size_t i = 0;
       i < predictionValueRects.size() && i < predictionValues.size(); i++) {
    Rectangle &r = predictionValueRects[i];
    bool hovered = CheckCollisionPointRec(mouse, r);
    DrawRectangleRounded(r, 0.15f, 6, Color{30, 27, 22, 255});
    DrawRectangleRoundedLines(r, 0.15f, 6, 2,
                              hovered ? gold : Color{110, 100, 80, 255});
    const char *label = TextFormat("%d", predictionValues[i]);
    int labelW = MeasureTextEx(labelFont, label, 20, 1).x;
    DrawTextEx(labelFont, label,
               {r.x + r.width / 2 - labelW / 2.0f, r.y + r.height / 2 - 12}, 20,
               1, textLight);
  }

  bool cancelHovered = CheckCollisionPointRec(mouse, predictionCancelRect);
  DrawRectangleRounded(predictionCancelRect, 0.1f, 6, Color{45, 30, 30, 255});
  DrawRectangleRoundedLines(predictionCancelRect, 0.1f, 6, 2,
                            cancelHovered ? gold : Color{140, 90, 90, 255});
  const char *cancelLabel = "Cancel (pick a different defense)";
  int cancelW = MeasureTextEx(labelFont, cancelLabel, 15, 1).x;
  DrawTextEx(
      labelFont, cancelLabel,
      {predictionCancelRect.x + predictionCancelRect.width / 2 - cancelW / 2.0f,
       predictionCancelRect.y + 12},
      15, 1, textLight);
}

void GameScene::handlePredictionMouse() {
  if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    return;
  Vector2 mouse = GetMousePosition();

  for (size_t i = 0;
       i < predictionValueRects.size() && i < predictionValues.size(); i++) {
    if (CheckCollisionPointRec(mouse, predictionValueRects[i])) {
      resolvePrediction(predictionValues[i]);
      return;
    }
  }
  if (CheckCollisionPointRec(mouse, predictionCancelRect)) {
    predictionPromptActive = false;
    pendingPredictionCard = nullptr;
  }
}

void GameScene::activateAction(int index) {
  if (index < 0 || index >= (int)actionButtons.size())
    return;
  if (combatDefensePending || isDiscardPromptActive())
    return;

  const std::string &title = actionButtons[index].title;

  cancelMovePicker();
  cancelTargetPicker();
  if (title != "Maneuver") {

    gameManager->finishManeuver();
  }
  if (title != "Attack") {
    attackModeActive = false;
    clearAttackRangePreview();
  }
  if (title != "Scheme")
    schemeModeActive = false;

  if (title == "End Turn") {
    if (!gameManager->endTurn()) {
      if (gameManager->getActionsRemaining() > 0) {
        pushLog(eventLog, "You still have an action left this turn.");
      } else if (gameManager->needsEndOfTurnDiscard()) {
        pushLog(eventLog, "Discard down to 7 cards before ending your turn.");
      }
    }
    refreshFromGameManager();
  } else if (title == "Maneuver") {

    if (!gameManager->performManeuver()) {
      pushLog(eventLog, "No actions left this turn.");
    } else {
      refreshFromGameManager();
      beginMovePicker();
    }
  } else if (title == "Attack") {

    attackModeActive = true;
    updateAttackRangePreview();
  } else if (title == "Scheme") {

    schemeModeActive = true;
  }
}

void GameScene::Update() {
  if (matchOver) {
    if (CheckCollisionPointRec(GetMousePosition(), resultMenuRect) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      scene->changeScene(ScenesType::mainScene);
    }
    return;
  }
  if (combatDefensePending) {
    if (predictionPromptActive) {
      handlePredictionMouse();

    } else {
      handleDefenseMouse();
    }
    return;
  }
  if (isCardChoicePromptActive()) {
    handleCardChoiceMouse();

    return;
  }
  if (isDiscardPromptActive()) {
    handleDiscardMouse();

    return;
  }
  if (isHandRevealPromptActive()) {
    handleHandRevealMouse();

    return;
  }
  handleMouse();
  handleKeyboard();
}

bool GameScene::isDiscardPromptActive() const {

  return gameManager->needsEndOfTurnDiscard();
}

void GameScene::Draw() {
  sw = (float)GetScreenWidth();
  sh = (float)GetScreenHeight();
  UpdateLayout();

  drawBackground();
  drawHeroPanel();
  drawOpponentsPanel();
  drawActionsPanel();
  drawDeck();
  drawBoard();

  drawHand();
  drawBottomBar();
  drawEventLog();
  if (combatDefensePending) {
    drawDefensePrompt();
    if (predictionPromptActive)
      drawPredictionPrompt();
  } else if (isCardChoicePromptActive()) {

    handRevealActive = false;
    drawCardChoicePrompt();
  } else if (isDiscardPromptActive())
    drawDiscardPrompt();
  else if (isHandRevealPromptActive())
    drawHandRevealPrompt();
  if (matchOver)
    drawResultScreen();
}

void GameScene::onGameStarted() {
  pushLog(eventLog, "The match begins.");
  refreshFromGameManager();
}

void GameScene::onTurnEnded(int newCurrentPlayer) {
  pushLog(eventLog,
          TextFormat("Turn passes to player %d.", newCurrentPlayer + 1));
  refreshFromGameManager();
}

void GameScene::onCardPlayed(Hero *player, Card *card) {
  if (player && card) {
    pushLog(eventLog, player->getName() + " played " + card->getName() + ".");
  }
  refreshFromGameManager();
}

void GameScene::onCardDiscarded(Hero *player, Card *card) {
  if (player && card) {
    pushLog(eventLog, player->getName() + " discarded " + card->getName() +
                          " (hand limit).");
  }
  refreshFromGameManager();
}

void GameScene::onManeuverPerformed(Hero *hero, bool deckWasEmpty) {
  if (hero) {
    if (deckWasEmpty) {
      pushLog(eventLog,
              hero->getName() +
                  "'s deck is empty -- fatigue! Everyone takes 2 damage.");
    } else {
      pushLog(eventLog, hero->getName() + " maneuvers: drew a card.");
    }
  }
  refreshFromGameManager();
}

void GameScene::onFighterRemoved(Fighter *fighter) {
  if (fighter) {
    pushLog(eventLog,
            fighter->getName() + " is defeated and removed from the board.");
  }
  refreshFromGameManager();
}

void GameScene::onFighterMoved(Fighter *fighter, int fromTileId, int toTileId) {
  if (fighter) {
    pushLog(eventLog,
            TextFormat("%s moved from tile %d to tile %d.",
                       fighter->getName().c_str(), fromTileId, toTileId));
  }
  refreshFromGameManager();
}

void GameScene::onFighterDamaged(Fighter *fighter, int amount) {
  if (fighter) {
    pushLog(eventLog, TextFormat("%s took %d damage.",
                                 fighter->getName().c_str(), amount));
  }
  refreshFromGameManager();
}

void GameScene::onCombatResolved(Fighter *attacker, Fighter *defender,
                                 Fighter *winner, Fighter *loser, int damage) {
  if (attacker && defender && winner) {
    if (damage > 0) {
      pushLog(eventLog, TextFormat("%s hits %s for %d in the exchange.",
                                   winner->getName().c_str(),
                                   loser->getName().c_str(), damage));
    } else {
      pushLog(eventLog,
              TextFormat("%s blocks %s's attack.", defender->getName().c_str(),
                         attacker->getName().c_str()));
    }
  }
  refreshFromGameManager();
}

void GameScene::onGameOver(Hero *winner) {
  cancelMovePicker();
  cancelTargetPicker();
  combatDefensePending = false;
  defenseCardOptions.clear();
  defenseHandRevealed = false;
  predictionPromptActive = false;
  pendingPredictionCard = nullptr;
  matchOver = true;
  winnerName = winner ? winner->getName() : "";
  handRevealActive = false;
  handRevealCards.clear();
  pushLog(eventLog,
          winner ? (winnerName + " wins the match!") : "The match has ended.");
  refreshFromGameManager();
}

void GameScene::onHandSeen(Hero *target, const std::vector<Card *> &hand) {
  if (!target)
    return;

  handRevealActive = true;
  handRevealOwnerName = target->getName();
  handRevealCards.clear();
  for (Card *c : hand) {
    if (c)
      handRevealCards.push_back(c);
  }

  pushLog(eventLog, "Revealed " + target->getName() + "'s hand.");
  refreshFromGameManager();
}