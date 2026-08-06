#pragma once
#include "view/scenes/Scene.h"
#include "view/ui/Button.h"
#include <string>
#include <vector>

class LoadGameScene : public Scene {
private:
  Font titleFont;
  Font slotTitleFont;
  Font slotSubFont;
  Font hintFont;

  Texture2D background;
  Texture2D iconCards;

  std::vector<Button> slotButtons;
  std::vector<bool> slotHasSave;  

  int selected{0};

  float sw;
  float sh;

  void UpdateLayout();
  void refreshSlotInfo(); 

  void drawBackground();
  void drawTitle();
  void drawSlots();
  void drawHint();

  void handleMouse();
  void handleKeyboard();

  void activateSlot(int index); 

public:
  LoadGameScene(AudioManager *, SceneManager *, TextureManager *, FontManager *);
  void Update() override;
  void Draw() override;
  void onEnter() override;
};
