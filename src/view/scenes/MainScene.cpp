#include "view/scenes/MainScene.h"
#include "controller/SceneManager.h"

std::vector<std::string> MainScene::wrapTextToWidth(
    const std::string &text, Font font, float fontSize,
    float maxWidth) // controll lines with width of panel(maxWidth)
{
  std::vector<std::string> lines;
  std::istringstream words(text); // words that are seperated with " "
  std::string word, currentLine;

  while (words >> word) // go one by one
  {
    std::string testLine = currentLine.empty()
                               ? word
                               : currentLine + " " + word; // checking for width
    float testWidth = MeasureTextEx(font, testLine.c_str(), fontSize, 1).x;

    if (testWidth > maxWidth &&
        !currentLine.empty()) // if smaller the panel width put it, if not then
                              // next line
    {
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

MainScene::MainScene(AudioManager *audioManager, SceneManager *sceneManager,
                     TextureManager *textureManager, FontManager *fontManager)
    : Scene(audioManager, sceneManager, textureManager,
            fontManager) // initializing managers and fonts and textures and
                         // buttons and quotes
{

  // getting font and putting in this variables
  cinzelBold = fontManager->getFont(FontID::CinzelBold, 72);
  cinzelSemiBold = fontManager->getFont(FontID::CinzelSemiBold, 52);
  cormoMedium = fontManager->getFont(FontID::CormorantGaramondMedium, 36);
  cormoRegular = fontManager->getFont(FontID::CormorantGaramondRegular, 34);
  // getting font and putting in this variables

  // putting filter on fonts
  SetTextureFilter(cinzelBold.texture, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(cinzelSemiBold.texture, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(cormoMedium.texture, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(cormoRegular.texture, TEXTURE_FILTER_BILINEAR);
  // putting filter on fonts

  // getting textures and putting in this variables
  background = texture->getTexture(TextureID::bg2);
  iconSword = texture->getTexture(TextureID::iconSword);
  iconExit = texture->getTexture(TextureID::iconExit);
  iconShield = texture->getTexture(TextureID::iconShield);
  iconBook = texture->getTexture(TextureID::iconBook);
  iconCards = texture->getTexture(TextureID::iconCards);
  logoTitle = texture->getTexture(TextureID::titleLogo);
  quoteBg = texture->getTexture(TextureID::quote);
  clickedButton = texture->getTexture(TextureID::clickedButton1);
  restButton = texture->getTexture(TextureID::restButton);
  // getting textures and putting in this variables

  // initializing buttons
  buttons = {{{},
              textureManager->getTexture(TextureID::iconSword),
              "START GAME",
              "Begin your journey"},
             {{},
              textureManager->getTexture(TextureID::iconBook),
              "LOAD GAME",
              "Continue a saved game"},
             {{},
              textureManager->getTexture(TextureID::iconCards),
              "COLLECTION",
              "View cards"},
             {{},
              textureManager->getTexture(TextureID::iconShield),
              "SETTINGS",
              "Preferences"},
             {{},
              textureManager->getTexture(TextureID::iconExit),
              "EXIT GAME",
              "Leave the realm"}};
  // initializing buttons

  // initializing quptes
  quotes = {
      {"King Arthur", "\"A king is measured by his final battle.\""},
      {"Sherlock Holmes", "\"Chance favors the prepared mind.\""},
      {"Dracula", "\"The night has endless patience.\""},
      {"Medusa", "\"One glance can end a lifetime.\""},
      {"Robin Hood", "\"The perfect shot is never hurried.\""},
      {"Achilles", "\"Even destiny must face my spear.\""},
      {"Sun Wukong", "\"Rules were made for slower minds.\""},
      {"Alice", "\"Wonder belongs to those who keep walking.\""},
      {"The Invisible Man", "\"You cannot fight what you cannot find.\""},
      {"Beowulf", "\"Great monsters create greater heroes.\""},
      {"Bloody Mary", "\"Every fear leaves a scar.\""},
      {"Jekyll & Hyde", "\"Every monster wears a familiar face.\""},
      {"Sinbad", "\"Every horizon hides another adventure.\""},
      {"Yennenga", "\"Honor is forged in impossible choices.\""},
      {"Moon Knight", "\"Justice arrives under many moons.\""},
      {"Tomoe Gozen", "\"Mercy is earned, never granted.\""},
      {"William Shakespeare", "\"Every duel deserves a memorable ending.\""},
      {"The Genie", "\"Power always asks for a price.\""},
      {"Little Red Riding Hood", "\"Every hunter was once the hunted.\""},
      {"Oda Nobunaga", "\"History remembers only the victorious.\""}};
  // initializing quptes

  sh = GetScreenHeight();
  sw = GetScreenWidth();
}

void MainScene::onEnter() // when entering a scene
{
  // playing background musics
  audio->playMusic(MusicID::mainBackgroundMusic, 0.45f, true);
  audio->playMusic(MusicID::fireBackgroundMusic, 0.10f, true);

  // initializing particles
  initParticles();

  // random index for quotes vector
  currentQuote = GetRandomValue(0, quotes.size() - 1);
}

void MainScene::Draw() // update layout, draw everything
{
  UpdateLayout();
  drawBackground();
  drawDarkLayer();
  drawParticles();
  drawLogoTitle();
  drawButtons();
  drawQuote();
  drawFooter();
}

void MainScene::Update() // particle, animation, navigate(enter)
{
  updateParticles(); // animation of particles

  handleMouse();    // navigate, enter
  handleKeyboard(); // navigate, enter

  timer += GetFrameTime();
  if (timer >= 16.0f) {
    timer = 0.0f;   // reset timer
    currentQuote++; // next quote
    if (currentQuote >= static_cast<int>(quotes.size())) {
      currentQuote = 0; // restart from the first quote
    }
  }

  // floating animation
  float t = GetTime();
  quoteTargetOffset = sinf(t * 0.45f) * 10.0f;
  quoteOffset += (quoteTargetOffset - quoteOffset) * 0.055f;
  // floating animation
}

void MainScene::activateButton(int index) // choose next scene
{
  switch (index) // switch on selected button
  {
  case 0:
    audio->playSound(SoundID::drawSword, 1.0f);
    scene->changeScene(ScenesType::heroSelection);
    break;
  case 1:
    audio->playSound(SoundID::clickForAll, 1.0f);
    scene->changeScene(ScenesType::loadGame);
    break;
  case 2:
    audio->playSound(SoundID::clickForAll, 1.0f);
    scene->changeScene(ScenesType::collection);
    break;
  case 3:
    audio->playSound(SoundID::clickForAll, 1.0f);
    scene->changeScene(ScenesType::setting);
    break;
  case 4:
    audio->playSound(SoundID::clickForAll, 1.0f);
    scene->requestQuit();
    break;
  }
}

void MainScene::handleKeyboard() // navigate, enter with keyboard
{
  if (IsKeyPressed(KEY_ENTER)) // enter
  {
    activateButton(selected);
  }

  for (auto &btn : buttons) // if mouse is on button then bye
  {
    if (CheckCollisionPointRec(GetMousePosition(), btn.rec)) {
      return;
    }
  }

  if (IsKeyPressed(KEY_DOWN)) // key down
  {
    selected = (selected + 1) % buttons.size();
  }

  if (IsKeyPressed(KEY_UP)) // key up
  {
    selected = (selected - 1 + buttons.size()) % buttons.size();
  }
}

void MainScene::handleMouse() // navigate, enter with mouse
{
  Vector2 mouse = GetMousePosition();
  for (size_t i = 0; i < buttons.size(); i++) // check all button
  {
    if (CheckCollisionPointRec(mouse, buttons[i].rec)) {

      selected = static_cast<int>(i);

      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        activateButton(selected);
      }
      break;
    }
  }
}

void MainScene::drawQuote() // drawing quoteBox, lines, text
{
  Rectangle panel = quotePanelRect;
  panel.y += quoteOffset;

  float padX = panel.width * 0.10f;
  float padY = panel.height * 0.16f;

  DrawRectangleRounded(panel, 0.12f, 24, Fade(BLACK, 0.23f)); // background box
  DrawRectangleRoundedLines(
      panel, 0.12f, 24, 2.0f,
      Fade(Color{185, 155, 90, 255}, 0.40f)); // golden box

  float lineLift = panel.height * 0.11f;
  float lineY = panel.y + panel.height - padY - lineLift;
  float textAreaTop = panel.y + padY;
  float textAreaWidth = panel.width - padX * 2.0f;

  float accentX = panel.x + padX * 0.45f;
  DrawLineEx({accentX, textAreaTop}, {accentX, lineY}, 2.0f,
             Fade(Color{205, 175, 105, 255}, 0.45f));

  const std::string &quoteText = quotes[currentQuote].text;
  std::vector<std::string> lines =
      wrapTextToWidth(quoteText, cormoRegular, quoteFontSize, textAreaWidth);

  float lineHeight = quoteFontSize * 1.32f;
  float totalTextHeight = lines.size() * lineHeight;

  float startY = textAreaTop + ((lineY - textAreaTop) - totalTextHeight) * 0.5f;

  for (size_t i = 0; i < lines.size(); i++) {
    DrawTextEx(cormoRegular, lines[i].c_str(),
               {panel.x + padX, startY + i * lineHeight}, quoteFontSize, 1,
               Color{224, 218, 202, 255});
  }

  DrawLineEx({panel.x + padX, lineY}, {panel.x + panel.width - padX, lineY}, 1,
             Fade(Color{180, 160, 120, 255}, 0.32f));

  std::string authorText = "-" + quotes[currentQuote].author;
  Vector2 aSize =
      MeasureTextEx(cormoMedium, authorText.c_str(), authorFontSize, 1);
  float authorAreaBottom = panel.y + panel.height - padY * 0.55f;
  float authorY = lineY + ((authorAreaBottom - lineY) - aSize.y) * 0.5f;

  DrawTextEx(cormoMedium, authorText.c_str(),
             {panel.x + panel.width - aSize.x - padX, authorY}, authorFontSize,
             1, Fade(Color{200, 182, 138, 255}, 0.90f));
}

void MainScene::drawBackground() // drawing background
{
  DrawTexturePro(background,
                 {0, 0, (float)background.width, (float)background.height},
                 {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                 {0, 0}, 0, WHITE);
}

void MainScene::drawDarkLayer() // drawing a gradiant black rect
{
  DrawRectangleGradientH(0, 0, GetScreenWidth(), GetScreenHeight(),
                         Fade(BLACK, 0.28f), Fade(BLACK, 0.02f));
}

void MainScene::drawFooter() // drawing footer, line and words
{

  DrawLineEx({sw * 0.03f, footerLineY}, {sw * 0.97f, footerLineY}, 1.5f,
             Fade(Color{190, 170, 120, 255}, 0.30f)); // drawing line

  const char *left = "UP/DOWN  Navigate";
  const char *center = "ENTER  Select";
  const char *right = "ESC  Quit";

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

void MainScene::drawLogoTitle() // drawing logo, title, subTitle
{
  DrawTexturePro(logoTitle,
                 {0, 0, (float)logoTitle.width, (float)logoTitle.height},
                 logoRect, {0, 0}, 0, WHITE); // drawing logo

  DrawTextEx(cinzelBold, "UNMATCHED", titlePos, titleFontSize, 2,
             Color{228, 205, 155, 255}); // drawing title

  DrawTextEx(cormoMedium, "STRATEGY CARD GAME", subtitlePos, subtitleFontSize,
             1, Color{165, 165, 160, 255}); // drawing subtitle
}

void MainScene::drawButtons() // drawing buttons, textures, texts, icons
{
  for (size_t i = 0; i < buttons.size(); i++) // loop on buttons
  {
    auto &btn = buttons[i];
    bool selectedButton = (selected == static_cast<int>(i));
    Rectangle rec = btn.rec;

    if (selectedButton) // bigger when selected
    {
      rec.x -= 4;
      rec.y -= 2;
      rec.width += 8;
      rec.height += 4;
    }

    Texture2D &bg = selectedButton ? clickedButton : restButton;

    DrawTexturePro(bg, {0, 0, (float)bg.width, (float)bg.height}, rec, {0, 0},
                   0, WHITE);

    if (selectedButton)
      DrawRectangleRounded(rec, 0.18f, 12,
                           Fade(Color{230, 190, 90, 255}, 0.08f));

    float iconSize = rec.height * 0.42f;
    Rectangle iconDst = {rec.x + rec.height * 0.30f,
                         rec.y + (rec.height - iconSize) / 2, iconSize,
                         iconSize};
    DrawTexturePro(btn.icon,
                   {0, 0, (float)btn.icon.width, (float)btn.icon.height},
                   iconDst, {0, 0}, 0, WHITE);

    float textX = iconDst.x + iconDst.width + rec.height * 0.35f;
    float titleSize = rec.height * 0.34f;
    float subtitleSize = rec.height * 0.175f;
    Color titleColor =
        selectedButton ? Color{235, 215, 170, 255} : Color{210, 198, 176, 255};
    Color subColor =
        selectedButton ? Color{190, 184, 176, 255} : Color{145, 145, 145, 255};

    Vector2 titleMeasure =
        MeasureTextEx(cinzelSemiBold, btn.title.c_str(), titleSize, 1);
    Vector2 subMeasure =
        MeasureTextEx(cormoRegular, btn.subTitle.c_str(), subtitleSize, 1);

    float totalTextHeight = titleMeasure.y + subMeasure.y + 6;
    float textStartY = rec.y + (rec.height - totalTextHeight) * 0.5f;

    DrawTextEx(cinzelSemiBold, btn.title.c_str(), {textX, textStartY},
               titleSize, 1, titleColor);
    DrawTextEx(cormoRegular, btn.subTitle.c_str(),
               {textX, textStartY + titleMeasure.y + 4}, subtitleSize, 1,
               subColor);
  }
}

void MainScene::drawParticles() // drawing particles
{
  float t = GetTime();

  for (auto &p : particles) // loop on all particles
  {

    float phase = p.radius * 2.7f + p.alpha * 0.13f;

    float swayAmplitude = 5.0f + p.radius * 3.0f;
    float x = p.position.x + sinf(t * 0.35f + phase) * swayAmplitude;
    float y = p.position.y;

    float twinkle = 0.5f + 0.5f * sinf(t * 0.6f + phase * 1.7f);
    float opacity = (p.alpha / 140.0f) * (0.3f + 0.7f * twinkle);

    DrawCircleV({x, y}, p.radius, Fade(Color{218, 200, 165, 255}, opacity));
  }
}

void MainScene::updateParticles() // changing position of every particles
{
  for (auto &p : particles) {
    p.position.x += p.velocity.x;
    p.position.y += p.velocity.y;
    if (p.position.y < -10) {
      p.position.y = (float)GetScreenHeight() + 10.0f;
      p.position.x = (float)GetRandomValue(0, GetScreenWidth());
    }
  }
}

void MainScene::initParticles() // creating particles
{
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

void MainScene::UpdateLayout() // responsive layout
{
  sw = (float)GetScreenWidth();
  sh = (float)GetScreenHeight();

  footerLineY = sh - 58;
  float menuWidth = sw * 0.34f;
  float menuX = sw * 0.055f;
  float cx = menuX + menuWidth * 0.5f;

  footerLineY = sh - 58.0f;

  float buttonH = sh * 0.10f;
  float spacing = sh * 0.015f;
  float menuH = buttons.size() * buttonH + (buttons.size() - 1) * spacing;
  float buttonBottomMargin = sh * 0.04f; // Margin above the footer line

  float menuY = footerLineY - menuH - buttonBottomMargin;

  for (size_t i = 0; i < buttons.size(); i++) {
    buttons[i].rec = {menuX, menuY + i * (buttonH + spacing), menuWidth,
                      buttonH};
  }

  float logoW = menuWidth * 0.50f;
  float logoScale = logoW / logoTitle.width;
  float topY = sh * 0.04f;

  logoRect = {cx - logoW * 0.5f, topY, logoW, logoTitle.height * logoScale};

  titleFontSize = sh * 0.082f;
  Vector2 tSize = MeasureTextEx(cinzelBold, "UNMATCHED", titleFontSize, 2);

  float logoVisualBottomInset = logoRect.height * 0.30f;
  float logoTitleGap = sh * 0.017f;

  titlePos = {cx - tSize.x * 0.5f, logoRect.y + logoRect.height -
                                       logoVisualBottomInset + logoTitleGap};

  subtitleFontSize = sh * 0.019f;
  Vector2 sSize =
      MeasureTextEx(cormoMedium, "STRATEGY CARD GAME", subtitleFontSize, 1);
  float titleSubtitleGap = sh * 0.008f;

  subtitlePos = {cx - sSize.x * 0.5f, titlePos.y + tSize.y + titleSubtitleGap};

  float quoteWidth = sw * 0.28f;
  float quoteH = sh * 0.14f;
  if (quoteH < sh * 0.10f)
    quoteH = sh * 0.10f;

  float quoteX = sw - quoteWidth - (sw * 0.035f);

  float quoteY = footerLineY - quoteH - (sh * 0.022f);

  quotePanelRect = {quoteX, quoteY, quoteWidth, quoteH};

  quoteFontSize = sh * 0.024f;
  authorFontSize = sh * 0.020f;
}