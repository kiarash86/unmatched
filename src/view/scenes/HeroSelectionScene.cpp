#include "view/scenes/HeroSelectionScene.h"
#include "controller/SceneManager.h"
//COLORS
static const Color kBgDark = {15, 15, 15, 255};
static const Color kPanelBg = {25, 25, 25, 235};
static const Color kGoldBright = {228, 205, 155, 255};
static const Color kGoldAccent = {205, 175, 105, 255}; 
static const Color kTextLight = {220, 220, 220, 255};
static const Color kTextMuted = {150, 150, 150, 255};
//COLORS

std::vector<std::string> HeroSelectionScene::wrapTextToWidth(
    const std::string &text, Font font, float fontSize,
    float maxWidth) 
{
  std::vector<std::string> lines;
  std::istringstream words(text);
  std::string word, currentLine;

  while (words >> word) {
    std::string testLine =
        currentLine.empty() ? word : currentLine + " " + word;
    float testWidth = MeasureTextEx(font, testLine.c_str(), fontSize, 1).x;

    if (testWidth > maxWidth && !currentLine.empty()) {
      lines.push_back(currentLine);
      currentLine = word;
    } else {
      currentLine = testLine;
    }
  }
  if (!currentLine.empty()) {
    lines.push_back(currentLine);
  }

  return lines;
}

HeroSelectionScene::HeroSelectionScene(AudioManager *audioManager,
                                       SceneManager *sceneManager,
                                       TextureManager *textureManager,
                                       FontManager *fontManager)
    : Scene(audioManager, sceneManager, textureManager, fontManager) {

  cinzelBold = fontManager->getFont(FontID::CinzelBold, 72);
  cinzelSemiBold = fontManager->getFont(FontID::CinzelSemiBold, 52);
  cormoMedium = fontManager->getFont(FontID::CormorantGaramondMedium, 36);
  cormoRegular = fontManager->getFont(FontID::CormorantGaramondRegular, 34);

  SetTextureFilter(cinzelBold.texture, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(cinzelSemiBold.texture, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(cormoMedium.texture, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(cormoRegular.texture, TEXTURE_FILTER_BILINEAR);

  iconFont = LoadFontEx("assets/fontawesome.otf", 64, codepoints, 4);
  SetTextureFilter(iconFont.texture, TEXTURE_FILTER_BILINEAR);

  int b = 0;
  iconHeart.assign(CodepointToUTF8(codepoints[0], &b), b);
  iconShield.assign(CodepointToUTF8(codepoints[1], &b), b);
  iconSwords.assign(CodepointToUTF8(codepoints[2], &b), b);
  iconShoe.assign(CodepointToUTF8(codepoints[3], &b), b);


  heroes.push_back(HeroInfoFactory::create(HeroList::Dracula));
  heroes.push_back(HeroInfoFactory::create(HeroList::SherlockHolms));
  for (auto &hero : heroes) {
    hero->wallpaper = LoadTexture(hero->wallpaperPath.c_str());
    hero->logo = LoadTexture(hero->logoPath.c_str());
  }

  players[0] = {-1, false};
  players[1] = {-1, false};
  currentPlayer = 0;
  selected = 0;

  sw = (float)GetScreenWidth();
  sh = (float)GetScreenHeight();
}

void HeroSelectionScene::onEnter() {
  initParticles();
}

void HeroSelectionScene::Update() {
  handleKeyboard();
  handleMouse();
  updateParticles();
}

void HeroSelectionScene::Draw() {
  UpdateLayout();
  drawBackground();
  drawParticles();
  drawTitle();
  drawTopBar();
  drawHeroList();
  drawInfoHero();
  drawFooter();
}

void HeroSelectionScene::handleKeyboard() {
  if (IsKeyPressed(KEY_DOWN)) {
    selected = (selected + 1) % (int)heroes.size();
  }
  if (IsKeyPressed(KEY_UP)) {
    selected = (selected - 1 + (int)heroes.size()) % (int)heroes.size();
  }

  if (IsKeyPressed(KEY_ENTER)) {
    if (currentPlayer < 2) {
      players[currentPlayer].selectedHeroIndex = selected;
      players[currentPlayer].isReady = true;
      currentPlayer++;
    }
  }
  if (IsKeyPressed(KEY_ESCAPE)) {
    if (currentPlayer > 0) {
      currentPlayer--;
      players[currentPlayer].isReady = false;
      players[currentPlayer].selectedHeroIndex = -1;
    }
  }
}

void HeroSelectionScene::handleMouse() {
  Vector2 mouse = GetMousePosition();
  for (size_t i = 0; i < cardRects.size(); i++) {
    if (CheckCollisionPointRec(mouse, cardRects[i])) {
      selected = (int)i;
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && currentPlayer < 2) {
        players[currentPlayer].selectedHeroIndex = selected;
        players[currentPlayer].isReady = true;
        currentPlayer++;
      }
      break;
    }
  }
}

void HeroSelectionScene::initParticles() {
  particles.clear();
  for (int i = 0; i < 70; i++) {
    Particle p;
    p.position = {(float)GetRandomValue(0, GetScreenWidth()),
                  (float)GetRandomValue(0, GetScreenHeight())};
    p.velocity = {GetRandomValue(-2, 2) / 24.0f,
                  GetRandomValue(-5, -2) / 24.0f};
    p.radius = GetRandomValue(12, 25) / 10.0f / 1.4f;
    p.alpha = (float)GetRandomValue(40, 100);
    particles.push_back(p);
  }
}

void HeroSelectionScene::updateParticles() {
  for (auto &p : particles) {
    p.position.x += p.velocity.x;
    p.position.y += p.velocity.y;
    if (p.position.y < -10) {
      p.position.y = (float)GetScreenHeight() + 10.0f;
      p.position.x = (float)GetRandomValue(0, GetScreenWidth());
    }
  }
}

void HeroSelectionScene::drawParticles() {
  float t = GetTime();
  for (auto &p : particles) {
    float phase = p.radius * 2.7f + p.alpha * 0.13f;
    float swayAmplitude = 5.0f + p.radius * 3.0f;
    float x = p.position.x + sinf(t * 0.35f + phase) * swayAmplitude;
    float y = p.position.y;

    float twinkle = 0.5f + 0.5f * sinf(t * 0.6f + phase * 1.7f);
    float opacity = (p.alpha / 140.0f) * (0.3f + 0.7f * twinkle);

    DrawCircleV({x, y}, p.radius, Fade(Color{218, 200, 165, 255}, opacity));
  }
}

void HeroSelectionScene::drawBackground() {
  auto currentHero = heroes[selected].get();

  ClearBackground(kBgDark);

  DrawRectangleGradientV(0, 0, (int)sw, (int)sh, currentHero->themeColor,
                        kBgDark);

  DrawTexturePro(currentHero->wallpaper,
                {0, 0, (float)currentHero->wallpaper.width,
                 (float)currentHero->wallpaper.height},
                {0, 0, sw, sh}, {0, 0}, 0, WHITE);

  DrawRectangleGradientH(0, 0, (int)sw, (int)sh, Fade(BLACK, 0.30f),
                        Fade(BLACK, 0.04f));
}

void HeroSelectionScene::drawTitle() {
  const char *title = "CHOOSE YOUR CHAMPION";
  Vector2 tSize = MeasureTextEx(cinzelBold, title, titleFontSize, 2);
  DrawTextEx(cinzelBold, title, titlePos, titleFontSize, 2, kGoldBright);

  float lineY = titlePos.y + tSize.y + sh * 0.012f;
  float lineWidth = sw * 0.16f;
  float cx = titlePos.x + tSize.x * 0.5f;
  DrawLineEx({cx - lineWidth * 0.5f, lineY}, {cx + lineWidth * 0.5f, lineY},
             2.0f, Fade(kGoldAccent, 0.55f));
}

void HeroSelectionScene::drawTopBar() {
  DrawRectangleRounded(topBarRect, 0.15f, 12, kPanelBg);
  DrawRectangleRoundedLines(topBarRect, 0.15f, 12, 2.0f, Fade(kGoldAccent, 0.6f));

  DrawTextEx(cinzelBold, "VS",
            {sw * 0.5f - MeasureTextEx(cinzelBold, "VS", vsFontSize, 2).x * 0.5f,
             topBarRect.y + topBarRect.height * 0.5f - vsFontSize * 0.5f},
            vsFontSize, 2, kGoldBright);

  float avatarSize = topBarRect.height * 0.68f;

  for (int p = 0; p < 2; p++) {
    bool onLeft = (p == 0);
    float pad = topBarRect.width * 0.04f;
    float pX = onLeft ? topBarRect.x + pad
                      : topBarRect.x + topBarRect.width - pad - avatarSize -
                            (sw * 0.20f);
    float pY = topBarRect.y + (topBarRect.height - avatarSize) * 0.5f;
    float textX = pX + avatarSize + pad * 0.6f;

    Color playerAccent = onLeft ? SKYBLUE : Color{230, 120, 110, 255};

    if (players[p].isReady) {
      auto pickedHero = heroes[players[p].selectedHeroIndex].get();
      DrawTexturePro(
          pickedHero->logo,
          {0, 0, (float)pickedHero->logo.width, (float)pickedHero->logo.height},
          {pX, pY, avatarSize, avatarSize}, {0, 0}, 0, WHITE);

      DrawTextEx(cinzelSemiBold, pickedHero->name.c_str(), {textX, pY},
                playerLabelFontSize, 1, kGoldBright);
      DrawTextEx(cormoMedium, pickedHero->role.c_str(),
                {textX, pY + playerLabelFontSize + 2}, playerSubFontSize, 1,
                kTextMuted);
      DrawTextEx(cormoMedium, "READY",
                {textX, pY + playerLabelFontSize + playerSubFontSize + 8},
                playerSubFontSize, 1, playerAccent);
    } else {
      DrawRectangleRounded({pX, pY, avatarSize, avatarSize}, 0.2f, 8,
                           Fade(DARKGRAY, 0.6f));
      DrawRectangleRoundedLines({pX, pY, avatarSize, avatarSize}, 0.2f, 8, 1.5f,
                                Fade(kGoldAccent, 0.4f));

      const char *pLabel = onLeft ? "Player 1" : "Player 2";
      bool isTurn = (currentPlayer == p);
      DrawTextEx(cinzelSemiBold, pLabel, {textX, pY},
                playerLabelFontSize, 1,
                isTurn ? kGoldBright : kTextMuted);
      DrawTextEx(cormoMedium, isTurn ? "Choosing..." : "Waiting...",
                {textX, pY + playerLabelFontSize + 4}, playerSubFontSize, 1,
                isTurn ? kTextLight : kTextMuted);
    }
  }
}

void HeroSelectionScene::drawHeroList() {
  for (int i = 0; i < (int)heroes.size(); i++) {
    Rectangle cardRect = cardRects[i];
    bool isSelected = (i == selected);

    Rectangle rec = cardRect;
    if (isSelected) {
      rec.x -= 4;
      rec.width += 8;
    }

    Color fill = isSelected ? Color{40, 45, 60, 220} : Color{15, 15, 15, 170};
    DrawRectangleRounded(rec, 0.12f, 10, fill);
    DrawRectangleRoundedLines(rec, 0.12f, 10, isSelected ? 2.5f : 1.5f,
                              isSelected ? kGoldAccent : Fade(kGoldAccent, 0.25f));

    DrawTextEx(isSelected ? cinzelSemiBold : cormoMedium,
              heroes[i]->name.c_str(),
              {rec.x + rec.width * 0.08f,
               rec.y + rec.height - cardNameFontSize * 1.6f},
              cardNameFontSize, 1, isSelected ? kGoldBright : kTextMuted);
  }
}

void HeroSelectionScene::drawStatBar(float x, float y, float width,
                                     const char *icon, int value, int max,
                                     Color color) {
  float barH = sh * 0.012f;
  float iconGap = statIconFontSize * 1.2f;

  DrawTextEx(iconFont, icon, {x, y - statIconFontSize * 0.6f}, statIconFontSize,
            0, color);

  Rectangle track = {x + iconGap, y - barH * 0.5f, width - iconGap, barH};
  DrawRectangleRounded(track, 0.5f, 6, Fade(WHITE, 0.12f));

  float ratio = max > 0 ? (float)value / (float)max : 0.0f;
  if (ratio > 1.0f) ratio = 1.0f;
  Rectangle fill = {track.x, track.y, track.width * ratio, barH};
  if (fill.width > 0.5f) {
    DrawRectangleRounded(fill, 0.5f, 6, color);
  }
}

void HeroSelectionScene::drawInfoHero() {
  auto hero = heroes[selected].get();
  Rectangle panel = infoPanelRect;

  DrawRectangleRounded(panel, 0.06f, 16, kPanelBg);
  DrawRectangleRoundedLines(panel, 0.06f, 16, 2.0f, Fade(kGoldAccent, 0.45f));

  float padX = panel.width * 0.05f;
  float padY = panel.height * 0.05f;
  float x = panel.x + padX;
  float y = panel.y + padY;
  float contentWidth = panel.width - padX * 2.0f;

  DrawTextEx(cinzelBold, hero->name.c_str(), {x, y}, nameFontSize, 1,
            kGoldBright);
  y += nameFontSize * 1.1f;

  DrawTextEx(cormoMedium, hero->role.c_str(), {x, y}, roleFontSize, 1,
            SKYBLUE);
  y += roleFontSize * 1.4f;

  DrawLineEx({x, y}, {x + contentWidth, y}, 1.5f, Fade(kGoldAccent, 0.35f));
  y += panel.height * 0.03f;

  std::vector<std::string> descLines =
      wrapTextToWidth(hero->desc, cormoRegular, descFontSize, contentWidth);
  float descLineHeight = descFontSize * 1.3f;
  for (auto &line : descLines) {
    DrawTextEx(cormoRegular, line.c_str(), {x, y}, descFontSize, 1,
              kTextLight);
    y += descLineHeight;
  }
  y += panel.height * 0.035f;

  float statGap = panel.height * 0.055f;
  float statBarWidth = contentWidth * 0.55f;

  drawStatBar(x, y, statBarWidth, iconHeart.c_str(), hero->hp, 10, RED);
  y += statGap;
  drawStatBar(x, y, statBarWidth, iconSwords.c_str(), hero->attack, 10,
             ORANGE);
  y += statGap;
  drawStatBar(x, y, statBarWidth, iconShield.c_str(), hero->defense, 10,
             kGoldBright);
  y += statGap;
  drawStatBar(x, y, statBarWidth, iconShoe.c_str(), hero->speed, 10, BLUE);
  y += statGap * 1.3f;

  DrawTextEx(cinzelSemiBold, "ABILITY", {x, y}, abilityTitleFontSize * 0.6f, 1,
            kGoldAccent);
  y += abilityTitleFontSize * 0.6f + panel.height * 0.012f;

  DrawTextEx(cinzelSemiBold, hero->abilityTitle.c_str(), {x, y},
            abilityTitleFontSize, 1, kGoldBright);
  y += abilityTitleFontSize * 1.2f;

  std::vector<std::string> abilityLines = wrapTextToWidth(
      hero->abilityDesc, cormoRegular, abilityDescFontSize, contentWidth);
  float abilityLineHeight = abilityDescFontSize * 1.3f;
  for (auto &line : abilityLines) {
    DrawTextEx(cormoRegular, line.c_str(), {x, y}, abilityDescFontSize, 1,
              kTextMuted);
    y += abilityLineHeight;
  }
}

void HeroSelectionScene::drawFooter() {
  DrawLineEx({sw * 0.03f, footerLineY}, {sw * 0.97f, footerLineY}, 1.5f,
             Fade(Color{190, 170, 120, 255}, 0.30f));

  const char *left = "UP/DOWN  Navigate";
  const char *center = "ENTER  Select";
  const char *right = "ESC  Back";

  Vector2 l = MeasureTextEx(cormoMedium, left, footerFontSize, 1);
  Vector2 c = MeasureTextEx(cormoMedium, center, footerFontSize, 1);
  Vector2 r = MeasureTextEx(cormoMedium, right, footerFontSize, 1);

  const float spacing = 55.0f;
  float totalWidth = l.x + spacing + c.x + spacing + r.x;
  float startX = (sw - totalWidth) * 0.5f;
  float y = footerLineY + 16;

  DrawTextEx(cormoMedium, left, {startX, y}, footerFontSize, 1,
            Color{185, 185, 180, 255});
  startX += l.x + spacing;
  DrawTextEx(cormoMedium, center, {startX, y}, footerFontSize, 1,
            Color{185, 185, 180, 255});
  startX += c.x + spacing;
  DrawTextEx(cormoMedium, right, {startX, y}, footerFontSize, 1,
            Color{185, 185, 180, 255});
}

void HeroSelectionScene::UpdateLayout() {
  sw = (float)GetScreenWidth();
  sh = (float)GetScreenHeight();

  footerLineY = sh - 58.0f;

  // title
  titleFontSize = sh * 0.05f;
  Vector2 tSize =
      MeasureTextEx(cinzelBold, "CHOOSE YOUR CHAMPION", titleFontSize, 2);
  titlePos = {sw * 0.5f - tSize.x * 0.5f, sh * 0.025f};

  // top bar (vs banner)
  float topBarY = titlePos.y + tSize.y + sh * 0.03f;
  topBarRect = {sw * 0.05f, topBarY, sw * 0.90f, sh * 0.14f};

  vsFontSize = sh * 0.045f;
  playerLabelFontSize = sh * 0.026f;
  playerSubFontSize = sh * 0.016f;

  // main content area (hero list + info panel)
  float contentTop = topBarRect.y + topBarRect.height + sh * 0.03f;
  float contentBottom = footerLineY - sh * 0.03f;
  float contentHeight = contentBottom - contentTop;

  listPanelRect = {sw * 0.05f, contentTop, sw * 0.22f, contentHeight};

  float gap = sw * 0.02f;
  infoPanelRect = {listPanelRect.x + listPanelRect.width + gap, contentTop,
                   sw * 0.95f - (listPanelRect.x + listPanelRect.width + gap),
                   contentHeight};

  // hero cards, evenly stacked inside the list panel
  cardRects.clear();
  int n = (int)heroes.size();
  if (n > 0) {
    float cardSpacing = sh * 0.015f;
    float cardHeight =
        (listPanelRect.height - (n - 1) * cardSpacing) / (float)n;
    for (int i = 0; i < n; i++) {
      cardRects.push_back({listPanelRect.x,
                           listPanelRect.y + i * (cardHeight + cardSpacing),
                           listPanelRect.width, cardHeight});
    }
  }
  cardNameFontSize = sh * 0.022f;

  // info panel font sizes
  nameFontSize = sh * 0.055f;
  roleFontSize = sh * 0.022f;
  descFontSize = sh * 0.019f;
  abilityTitleFontSize = sh * 0.028f;
  abilityDescFontSize = sh * 0.018f;
  statIconFontSize = sh * 0.026f;
}

HeroSelectionScene::~HeroSelectionScene() {
  for (auto &hero : heroes) {
    UnloadTexture(hero->wallpaper);
    UnloadTexture(hero->logo);
  }
  UnloadFont(iconFont);
}