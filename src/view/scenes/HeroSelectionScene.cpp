#include "view/scenes/HeroSelectionScene.h"
#include "controller/PlayerSelectionManager.h"
#include "controller/SceneManager.h"

HeroSelectionScene::~HeroSelectionScene() {}

TextureID HeroSelectionScene::bgForHero(HeroList id) {
  switch (id) {
  case HeroList::Dracula:
    return TextureID::bgDracula;
  case HeroList::SherlockHolms:
    return TextureID::bgSherlock;
  case HeroList::InvisibleMan:
    return TextureID::bgInvisibleMan;
  default:
    return TextureID::bg2;
  }
}

TextureID HeroSelectionScene::normalButtonForHero(HeroList id) {
  switch (id) {
  case HeroList::Dracula:
    return TextureID::draculaButtonNormal;
  case HeroList::SherlockHolms:
    return TextureID::sherlockButtonNormal;
  case HeroList::InvisibleMan:
    return TextureID::invisibleManButtonNormal;
  default:
    return TextureID::draculaButtonNormal;
  }
}

TextureID HeroSelectionScene::hoveredButtonForHero(HeroList id) {
  switch (id) {
  case HeroList::Dracula:
    return TextureID::draculaButtonHovered;
  case HeroList::SherlockHolms:
    return TextureID::sherlockButtonHovered;
  case HeroList::InvisibleMan:
    return TextureID::invisibleManButtonHovered;
  default:
    return TextureID::draculaButtonHovered;
  }
}

SoundID HeroSelectionScene::soundForHero(HeroList id) {
  switch (id) {
  case HeroList::Dracula:
    return SoundID::draculaSpeech;
  case HeroList::SherlockHolms:
    return SoundID::sherlockSpeech;
  case HeroList::InvisibleMan:
    return SoundID::invisiblemanSpeech;
  default:
    return SoundID::draculaSpeech;
  }
}

HeroSelectionScene::HeroSelectionScene(AudioManager *audioManager, SceneManager *sceneManager,TextureManager *textureManager, FontManager *fontManager)
    : Scene(audioManager, sceneManager, textureManager, fontManager) {

  const HeroList hrs[] = {HeroList::Dracula, HeroList::SherlockHolms, HeroList::InvisibleMan}; // heroes

  // font

  cinzelBold = fontManager->getFont(FontID::CinzelBold, 70);

  cinzelSemiBold = fontManager->getFont(FontID::CinzelSemiBold, 42);

  cormoMedium = fontManager->getFont(FontID::CormorantGaramondMedium, 32);

  cormoRegular = fontManager->getFont(FontID::CormorantGaramondRegular, 30);

  // font
  SetTextureFilter(cinzelBold.texture, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(cinzelSemiBold.texture, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(cormoMedium.texture, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(cormoRegular.texture, TEXTURE_FILTER_BILINEAR);

  for (const HeroList &id : hrs) {
    auto hero = HeroInfoFactory::create(id);

    hero->wallpaper = texture->getTexture(bgForHero(id));

    hero->normalButton = texture->getTexture(normalButtonForHero(id));
    hero->hoveredButton = texture->getTexture(hoveredButtonForHero(id));

    heroes.push_back(std::move(hero));
    heroIds.push_back(id);
  }

  sw = (float)GetScreenWidth();
  sh = (float)GetScreenHeight();
}

void HeroSelectionScene::onEnter() {

  audio->playMusic(MusicID::mainBackgroundMusic, 0.40f, true);

  PlayerSelectionManager::instance().reset();

  currentPlayerIndex = 0;

  selectedHero = 0;

  vsAIMode = false;
  gameModeLocked = false;

  pendingVoiceAction = PendingVoiceAction::none;

  audio->playSound(SoundID::player1Choose, 1.0f);
}

void HeroSelectionScene::Draw() {
  UpdateLayout();

  drawBackground();

  drawTitle();

  drawBackButton();

  drawModeToggle();

  drawHeroList();

  drawInfoPanel();
}

void HeroSelectionScene::Update() {
  UpdateLayout();

  if (pendingVoiceAction != PendingVoiceAction::none &&
      !audio->isSoundPlaying(heroVoiceBeingWaitedOn)) {
    if (pendingVoiceAction == PendingVoiceAction::Player2) {
      audio->playSound(SoundID::player2Choose, 1.0f);
    } else if (pendingVoiceAction == PendingVoiceAction::startTransition) {
      pendingVoiceAction = PendingVoiceAction::none;
      scene->changeScene(ScenesType::game);
      return;
    }

    pendingVoiceAction = PendingVoiceAction::none;
  }

  handleMouse();

  handleKeyboard();
}

void HeroSelectionScene::UpdateLayout() {
  sw = (float)GetScreenWidth();
  sh = (float)GetScreenHeight();

  heroNameFontSize = sh * 0.028f;
  heroRoleFontSize = sh * 0.020f;

  panelTitleFontSize = sh * 0.024f;
  descriptionFontSize = sh * 0.019f;

  statFontSize = sh * 0.022f;

  buttonFontSize = sh * 0.021f;

  Texture2D titleTex = texture->getTexture(TextureID::titleHeroSelection);

  float titleImgWidth = sw * 0.40f;
  float titleImgHeight =
      titleTex.width > 0
          ? titleImgWidth * (float)titleTex.height / (float)titleTex.width
          : 0.0f;

  titleImageRect = {sw * 0.5f - titleImgWidth * 0.5f, sh * 0.02f, titleImgWidth,
                    titleImgHeight};

  backButtonRect = {sw * 0.03f, sh * 0.045f, sw * 0.11f, sh * 0.055f};

  float modeBtnWidth = sw * 0.15f;
  float modeBtnHeight = sh * 0.055f;
  float modeBtnGap = sw * 0.01f;

  twoPlayerModeRect = {sw * 0.97f - modeBtnWidth, sh * 0.045f, modeBtnWidth,
                       modeBtnHeight};
  onePlayerModeRect = {twoPlayerModeRect.x - modeBtnGap - modeBtnWidth,
                       sh * 0.045f, modeBtnWidth, modeBtnHeight};

  float listWidth = sw * 0.24f;
  float cardSpacing = sh * 0.018f;

  std::vector<float> cardHeights;
  float totalCardsHeight = 0.0f;

  for (size_t i = 0; i < heroes.size(); i++) {
    Texture2D t = heroes[i]->normalButton;

    float aspect = (t.width > 0) ? (float)t.height / (float)t.width : 0.32f;
    float h = listWidth * aspect;

    cardHeights.push_back(h);
    totalCardsHeight += h;
  }

  if (heroes.size() > 1)
    totalCardsHeight += cardSpacing * (float)(heroes.size() - 1);

  float columnsY = titleImageRect.y + titleImageRect.height + sh * 0.02f;
  float columnsHeight = sh * 0.87f - columnsY;

  heroListRect = {sw * 0.03f, columnsY, listWidth,
                  totalCardsHeight > 0.0f ? totalCardsHeight : sh * 0.11f};

  paginationRect = {heroListRect.x,
                    heroListRect.y + heroListRect.height + sh * 0.015f,
                    heroListRect.width, sh * 0.03f};

  float infoPanelHeight = sh * 0.62f;

  if (infoPanelHeight > columnsHeight)
    infoPanelHeight = columnsHeight;

  infoPanelRect = {sw * 0.735f, columnsY, sw * 0.235f, infoPanelHeight};

  heroCardRects.clear();

  if (!heroes.empty()) {
    float y = heroListRect.y;

    for (size_t i = 0; i < heroes.size(); i++) {
      heroCardRects.push_back(
          {heroListRect.x, y, heroListRect.width, cardHeights[i]});

      y += cardHeights[i] + cardSpacing;
    }
  }
}

void HeroSelectionScene::drawBackground() {
  ClearBackground(BLACK);

  Texture2D bg = (!heroes.empty() && selectedHero < (int)heroes.size())
                     ? heroes[selectedHero]->wallpaper
                     : texture->getTexture(TextureID::bg2);

  DrawTexturePro(bg, {0, 0, (float)bg.width, (float)bg.height}, {0, 0, sw, sh},
                 {0, 0}, 0, WHITE);
}

void HeroSelectionScene::drawTitle() {
  Texture2D &titleTex = texture->getTexture(TextureID::titleHeroSelection);

  DrawTexturePro(titleTex,
                 {0, 0, (float)titleTex.width, (float)titleTex.height},
                 titleImageRect, {0, 0}, 0, WHITE);
}

void HeroSelectionScene::drawBackButton() {
  Vector2 mouse = GetMousePosition();

  bool hover = CheckCollisionPointRec(mouse, backButtonRect);

  Color bg = hover ? Color{45, 48, 58, 235} : Color{26, 28, 36, 215};

  DrawRectangleRounded(backButtonRect, 0.18f, 12, bg);

  DrawRectangleRoundedLines(backButtonRect, 0.18f, 12, 2,
                            Color{195, 160, 90, 255});

  DrawTextEx(cormoMedium, "BACK",
             {backButtonRect.x + 18, backButtonRect.y +
                                         backButtonRect.height / 2 -
                                         (float)cormoMedium.baseSize / 2},
             buttonFontSize, 1, RAYWHITE);
}

void HeroSelectionScene::drawModeToggle() {
  Vector2 mouse = GetMousePosition();

  struct ModeButton {
    Rectangle rect;
    const char *label;
    bool active;
  };

  ModeButton buttons[2] = {
      {onePlayerModeRect, "1 PLAYER (VS AI)", vsAIMode},
      {twoPlayerModeRect, "2 PLAYERS", !vsAIMode},
  };

  for (auto &btn : buttons) {
    bool hover =
        !gameModeLocked && CheckCollisionPointRec(mouse, btn.rect);

    Color bg;
    Color border;

    if (btn.active) {
      bg = Color{90, 60, 20, 235};
      border = GOLD;
    } else if (hover) {
      bg = Color{45, 48, 58, 235};
      border = Color{195, 160, 90, 255};
    } else {
      bg = Color{26, 28, 36, 215};
      border = Color{120, 110, 90, 180};
    }

    if (gameModeLocked && !btn.active) {
      bg = Fade(bg, 0.5f);
      border = Fade(border, 0.5f);
    }

    DrawRectangleRounded(btn.rect, 0.18f, 12, bg);
    DrawRectangleRoundedLines(btn.rect, 0.18f, 12, 2, border);

    Vector2 textSize =
        MeasureTextEx(cormoMedium, btn.label, buttonFontSize * 0.75f, 1);

    DrawTextEx(cormoMedium, btn.label,
               {btn.rect.x + btn.rect.width / 2 - textSize.x / 2,
                btn.rect.y + btn.rect.height / 2 - textSize.y / 2},
               buttonFontSize * 0.75f, 1, btn.active ? GOLD : RAYWHITE);
  }
}

void HeroSelectionScene::drawDifficultyStars(float x, float y, int rating, int maxRating) {
  float spacing = statFontSize * 0.95f;
  float radius = statFontSize * 0.32f;

  for (int i = 0; i < maxRating; i++) {
    bool filled = i < rating;

    float cx = x + radius + (float)i * spacing;
    float cy = y + radius;

    if (filled) {
      DrawCircle((int)cx, (int)cy, radius, GOLD);
    } else {
      DrawCircleLines((int)cx, (int)cy, radius, Fade(GOLD, 0.5f));
    }
  }
}

void HeroSelectionScene::drawHeroList() {
  Vector2 mouse = GetMousePosition();

  for (size_t i = 0; i < heroCardRects.size(); i++) {
    Rectangle r = heroCardRects[i];

    bool hover = CheckCollisionPointRec(mouse, r);

    bool active = (int)i == selectedHero;

    HeroList id = (i < heroIds.size()) ? heroIds[i] : HeroList::Dracula;
    bool taken = PlayerSelectionManager::instance().isHeroTaken(id);

    InfoHero *cardHero = heroes[i].get();

    Texture2D buttonTex = (!taken && (hover || active))
                              ? cardHero->hoveredButton
                              : cardHero->normalButton;

    DrawTexturePro(buttonTex,
                   {0, 0, (float)buttonTex.width, (float)buttonTex.height}, r,
                   {0, 0}, 0, WHITE);

    if (taken) {
      DrawRectangleRounded(r, 0.08f, 8, Fade(BLACK, 0.55f));

      const char *label = "SELECTED";
      Vector2 s = MeasureTextEx(cormoMedium, label, heroRoleFontSize, 1);

      DrawTextEx(cormoMedium, label,
                 {r.x + r.width * 0.5f - s.x * 0.5f,
                  r.y + r.height * 0.5f - s.y * 0.5f},
                 heroRoleFontSize, 1, GOLD);
    }
  }
}

//TODO: ATTRIBUTES
void HeroSelectionScene::drawInfoPanel() {
  if (heroes.empty())
    return;

  InfoHero *hero = heroes[selectedHero].get();

  Rectangle infoContentRect = {infoPanelRect.x + infoPanelRect.width * 0.032f,
                               infoPanelRect.y + infoPanelRect.height * 0.027f,
                               infoPanelRect.width * 0.937f,
                               infoPanelRect.height * 0.946f};

  DrawRectangleRec(infoContentRect, Fade(BLACK, 0.75f));

  Texture2D frameTex = texture->getTexture(TextureID::frame);

  DrawTexturePro(frameTex,
                 {0, 0, (float)frameTex.width, (float)frameTex.height},
                 infoPanelRect, {0, 0}, 0, WHITE);

  float x = infoContentRect.x + 25;
  float y = infoContentRect.y + 25;

 
  DrawTextEx(cinzelBold, hero->name.c_str(), {x, y}, heroNameFontSize, 2, GOLD);

  y += heroNameFontSize + 10;

  
  DrawTextEx(cormoMedium, hero->role.c_str(), {x, y}, heroRoleFontSize, 1,
             LIGHTGRAY);

  y += heroRoleFontSize + 16;


  if (!hero->desc.empty()) {
    std::vector<std::string> descLines;
    std::istringstream words(hero->desc);
    std::string word, line;
    float maxWidth = infoContentRect.width - 50;

    while (words >> word) {
      std::string test = line.empty() ? word : line + " " + word;
      float w =
          MeasureTextEx(cormoRegular, test.c_str(), descriptionFontSize, 1).x;

      if (w > maxWidth && !line.empty()) {
        descLines.push_back(line);
        line = word;
      } else {
        line = test;
      }
    }
    if (!line.empty())
      descLines.push_back(line);

    for (auto &l : descLines) {
      DrawTextEx(cormoRegular, l.c_str(), {x, y}, descriptionFontSize, 1,
                 Color{190, 185, 175, 255});
      y += descriptionFontSize * 1.3f;
    }

    y += 14;
  }

 
  DrawTextEx(cinzelSemiBold, "DIFFICULTY", {x, y}, panelTitleFontSize, 1,
             RAYWHITE);

  drawDifficultyStars(
      x + MeasureTextEx(cinzelSemiBold, "DIFFICULTY", panelTitleFontSize, 1).x +
          14,
      y + panelTitleFontSize * 0.15f, hero->difficulty);

  y += panelTitleFontSize + 34;


  DrawTextEx(cinzelSemiBold, "ATTRIBUTES", {x, y}, panelTitleFontSize, 1,
             RAYWHITE);

  y += panelTitleFontSize + 18;

  y += 35;

  y += 35;

  y += 35;

  y += 55;

  DrawTextEx(cinzelSemiBold, "ABILITY", {x, y}, panelTitleFontSize, 1,
             RAYWHITE);

  y += panelTitleFontSize + 15;

  Rectangle abilityRect = {x, y, infoContentRect.width - 50, 120};

  DrawRectangleRounded(abilityRect, 0.05f, 8, Color{35, 38, 45, 255});

  DrawRectangleRoundedLines(abilityRect, 0.05f, 8, 1, Fade(GOLD, 0.4f));

  DrawTextEx(cinzelSemiBold, hero->abilityTitle.c_str(),
             {abilityRect.x + 15, abilityRect.y + 15}, statFontSize, 1, GOLD);

  std::vector<std::string> abilityLines;
  {
    std::istringstream words(hero->abilityDesc);
    std::string word, line;
    float maxWidth = abilityRect.width - 30;

    while (words >> word) {
      std::string test = line.empty() ? word : line + " " + word;
      float w =
          MeasureTextEx(cormoRegular, test.c_str(), descriptionFontSize, 1).x;

      if (w > maxWidth && !line.empty()) {
        abilityLines.push_back(line);
        line = word;
      } else {
        line = test;
      }
    }
    if (!line.empty())
      abilityLines.push_back(line);
  }

  float lineY = abilityRect.y + 50;
  for (auto &l : abilityLines) {
    DrawTextEx(cormoRegular, l.c_str(), {abilityRect.x + 15, lineY},
               descriptionFontSize, 1, LIGHTGRAY);
    lineY += descriptionFontSize * 1.3f;
  }
}

void HeroSelectionScene::handleMouse() {

  Vector2 mouse = GetMousePosition(); // position of mouse

  bool overInteractive = false; 

  for (size_t i = 0; i < heroCardRects.size(); i++) {
    if (CheckCollisionPointRec(mouse, heroCardRects[i])) {

      HeroList id = (i < heroIds.size()) ? heroIds[i] : HeroList::Dracula;

      if (PlayerSelectionManager::instance().isHeroTaken(id))
        continue;

      overInteractive = true;
      selectedHero = (int)i;

      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        confirmSelection();
      }
    }
  }

  if (CheckCollisionPointRec(mouse, backButtonRect)) {
    overInteractive = true;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      scene->changeScene(ScenesType::mainScene);
    }
  }

  if (!gameModeLocked) {
    if (CheckCollisionPointRec(mouse, onePlayerModeRect)) {
      overInteractive = true;

      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !vsAIMode) {
        vsAIMode = true;
        PlayerSelectionManager::instance().setVsAI(vsAIMode);
      }
    } else if (CheckCollisionPointRec(mouse, twoPlayerModeRect)) {
      overInteractive = true;

      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && vsAIMode) {
        vsAIMode = false;
        PlayerSelectionManager::instance().setVsAI(vsAIMode);
      }
    }
  }

  SetMouseCursor(overInteractive ? MOUSE_CURSOR_POINTING_HAND
                                 : MOUSE_CURSOR_DEFAULT);
}

void HeroSelectionScene::handleKeyboard() {
  if (heroes.empty()) // no hero? then go away
  {
    return;
  }

  if (IsKeyPressed(KEY_DOWN)) {
    int next = selectedHero;

    for (int t = 0; t < (int)heroes.size(); t++) {
      next++;

      if (next >= (int)heroes.size())
        next = 0;

      HeroList id =
          (next < (int)heroIds.size()) ? heroIds[next] : HeroList::Dracula;
      if (!PlayerSelectionManager::instance().isHeroTaken(id)) {
        selectedHero = next;
        break;
      }
    }
  }

  if (IsKeyPressed(KEY_UP)) {
    int next = selectedHero;

    for (int t = 0; t < (int)heroes.size(); t++) {
      next--;

      if (next < 0)
        next = (int)heroes.size() - 1;

      HeroList id =
          (next < (int)heroIds.size()) ? heroIds[next] : HeroList::Dracula;
      if (!PlayerSelectionManager::instance().isHeroTaken(id)) {
        selectedHero = next;
        break;
      }
    }
  }

  if (IsKeyPressed(KEY_ENTER)) // ok so this is your choice for hero
  {
    confirmSelection();
  }

  if (IsKeyPressed(KEY_ESCAPE)) // return to main menu
  {
    scene->changeScene(ScenesType::mainScene);
  }
}

void HeroSelectionScene::confirmSelection() {
  if (heroes.empty())
    return;

  if (currentPlayerIndex >=
      totalPlayers) // everyone select their hero? then just return
  {
    return;
  }

  HeroList chosenId = heroIds[selectedHero];

  if (PlayerSelectionManager::instance().isHeroTaken(
          chosenId)) // on a selected hero? return
  {
    return;
  }

  SoundID chosenSound = soundForHero(chosenId);
  audio->playSound(chosenSound, 1.0f);

  Player player(chosenId);
  PlayerSelectionManager::instance().addPlayer(player);

  currentPlayerIndex++;

  gameModeLocked = true;

  if (currentPlayerIndex < totalPlayers && vsAIMode) {

    moveToNextAvailableHero();

    HeroList aiChosenId = heroIds[selectedHero];

    Player aiPlayer(aiChosenId);
    PlayerSelectionManager::instance().addPlayer(aiPlayer);

    currentPlayerIndex++;
  }

  if (currentPlayerIndex >= totalPlayers) // next scene or next choosing hero?
  {

    pendingVoiceAction = PendingVoiceAction::startTransition;
    heroVoiceBeingWaitedOn = chosenSound;
  } else {

    pendingVoiceAction = PendingVoiceAction::Player2;
    heroVoiceBeingWaitedOn = chosenSound;

    moveToNextAvailableHero();
  }
}

void HeroSelectionScene::moveToNextAvailableHero() {
  for (int i = 0; i < (int)heroIds.size(); i++) {
    if (!PlayerSelectionManager::instance().isHeroTaken(heroIds[i])) {
      selectedHero = i;
      return;
    }
  }
}