#include "view/scenes/LoadGameScene.h"
#include "controller/GameManager.h"
#include "controller/SceneManager.h"
#include "model/LoadGameSelection.h"

LoadGameScene::LoadGameScene(AudioManager *audioManager, SceneManager *sceneManager,
                             TextureManager *textureManager, FontManager *fontManager)
    : Scene(audioManager, sceneManager, textureManager, fontManager) {

  titleFont = fontManager->getFont(FontID::CinzelBold, 56);
  slotTitleFont = fontManager->getFont(FontID::CinzelSemiBold, 32);
  slotSubFont = fontManager->getFont(FontID::CormorantGaramondRegular, 26);
  hintFont = fontManager->getFont(FontID::CormorantGaramondRegular, 22);

  background = texture->getTexture(TextureID::bg2);
  iconCards = texture->getTexture(TextureID::iconCards);

  slotButtons.resize(GameManager::kSaveSlotCount);
  slotHasSave.resize(GameManager::kSaveSlotCount, false);

  sw = (float)GetScreenWidth();
  sh = (float)GetScreenHeight();
}

void LoadGameScene::refreshSlotInfo() {
  for (int i = 0; i < GameManager::kSaveSlotCount; i++) {
    slotHasSave[i] = GameManager::hasSave(i + 1); 
  }
}

void LoadGameScene::onEnter() {
  refreshSlotInfo(); 
  selected = 0;
}

void LoadGameScene::UpdateLayout() {
  sw = (float)GetScreenWidth();
  sh = (float)GetScreenHeight();

  float slotWidth = sw * 0.5f;
  float slotHeight = sh * 0.14f;
  float gap = sh * 0.03f;
  float startX = (sw - slotWidth) / 2.0f;

  float totalHeight = slotHeight * GameManager::kSaveSlotCount +
                      gap * (GameManager::kSaveSlotCount - 1);
  float startY = sh * 0.5f - totalHeight / 2.0f;

  for (int i = 0; i < GameManager::kSaveSlotCount; i++) {
    slotButtons[i].rec = {startX, startY + i * (slotHeight + gap), slotWidth,
                          slotHeight};
    slotButtons[i].icon = iconCards;
    slotButtons[i].title = "SLOT " + std::to_string(i + 1);
    slotButtons[i].subTitle = slotHasSave[i] ? "Continue saved game" : "Empty";
  }
}

void LoadGameScene::Draw() {
  UpdateLayout();
  drawBackground();
  drawTitle();
  drawSlots();
  drawHint();
}

void LoadGameScene::Update() {
  handleMouse();
  handleKeyboard();
}

void LoadGameScene::drawBackground() {
  DrawTexturePro(background, {0, 0, (float)background.width, (float)background.height},
                 {0, 0, sw, sh}, {0, 0}, 0, WHITE);
  DrawRectangle(0, 0, (int)sw, (int)sh, Fade(BLACK, 0.55f));
}

void LoadGameScene::drawTitle() {
  const char *title = "LOAD GAME";
  Vector2 size = MeasureTextEx(titleFont, title, titleFont.baseSize, 2);
  DrawTextEx(titleFont, title, {(sw - size.x) / 2.0f, sh * 0.12f}, (float)titleFont.baseSize,
             2, Color{212, 175, 55, 255});
}

void LoadGameScene::drawSlots() {
  for (size_t i = 0; i < slotButtons.size(); i++) {
    const Button &btn = slotButtons[i];
    bool isSelected = (selected == (int)i);
    bool occupied = slotHasSave[i];

    Color boxColor = occupied ? Fade(BLACK, 0.45f) : Fade(BLACK, 0.25f);
    Color borderColor = isSelected ? Color{212, 175, 55, 255}
                                   : Fade(Color{212, 175, 55, 255}, occupied ? 0.5f : 0.2f);

    DrawRectangleRounded(btn.rec, 0.15f, 16, boxColor);
    DrawRectangleRoundedLines(btn.rec, 0.15f, 16, isSelected ? 3.0f : 1.5f, borderColor);

    Color textColor = occupied ? Color{225, 220, 210, 255} : Fade(WHITE, 0.35f);
    Color subColor = occupied ? Color{150, 145, 135, 255} : Fade(WHITE, 0.25f);

    Vector2 titlePos = {btn.rec.x + 30, btn.rec.y + btn.rec.height * 0.22f};
    Vector2 subPos = {btn.rec.x + 30, btn.rec.y + btn.rec.height * 0.55f};

    DrawTextEx(slotTitleFont, btn.title.c_str(), titlePos, (float)slotTitleFont.baseSize, 1,
               textColor);
    DrawTextEx(slotSubFont, btn.subTitle.c_str(), subPos, (float)slotSubFont.baseSize, 1,
               subColor);
  }
}

void LoadGameScene::drawHint() {
  const char *hint = "Enter: Continue     Esc: Back to Menu";
  Vector2 size = MeasureTextEx(hintFont, hint, hintFont.baseSize, 1);
  DrawTextEx(hintFont, hint, {(sw - size.x) / 2.0f, sh * 0.9f}, (float)hintFont.baseSize, 1,
             Fade(WHITE, 0.6f));
}

void LoadGameScene::handleMouse() {
  Vector2 mouse = GetMousePosition();
  for (size_t i = 0; i < slotButtons.size(); i++) {
    if (CheckCollisionPointRec(mouse, slotButtons[i].rec)) {
      selected = (int)i;
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        activateSlot(selected);
      }
      break;
    }
  }
}

void LoadGameScene::handleKeyboard() {
  if (IsKeyPressed(KEY_DOWN)) {
    selected = (selected + 1) % (int)slotButtons.size();
  }
  if (IsKeyPressed(KEY_UP)) {
    selected = (selected - 1 + (int)slotButtons.size()) % (int)slotButtons.size();
  }
  if (IsKeyPressed(KEY_ENTER)) {
    activateSlot(selected);
  }
  if (IsKeyPressed(KEY_ESCAPE)) {
    audio->playSound(SoundID::clickForAll, 1.0f);
    scene->changeScene(ScenesType::mainScene);
  }
}

void LoadGameScene::activateSlot(int index) {
  if (index < 0 || index >= (int)slotHasSave.size() || !slotHasSave[index]) {

    return;
  }
  audio->playSound(SoundID::clickForAll, 1.0f);
  LoadGameSelection::instance().requestSlot(index + 1); 
  scene->changeScene(ScenesType::game);
}
