#pragma once                   // once per program
#include "view/scenes/Scene.h" // father 
#include "factory/heroInfoFactory.h" // factory of info
#include "model/InfoHero.h" // model of info0
#include <sstream> // word wrap
#include "view/ui/Particle.h" // particle
class HeroSelectionScene : public Scene // hero selection menu
{
private:
  struct PlayerState // player selection state
  {
    int selectedHeroIndex;
    bool isReady;
  };

  // FONTS
  Font cinzelBold;
  Font cinzelSemiBold;
  Font cormoMedium;
  Font cormoRegular;
  Font iconFont;
  // FONTS

  // HEROES
  std::vector<std::unique_ptr<InfoHero>> heroes;
  // HEROES

  // PLAYERS
  PlayerState players[2];
  int currentPlayer{0};
  int selected{0};
  // PLAYERS

  // ICONS
  int codepoints[4] = {
      0xf004, // heart
      0xf132, // shield
      0xf6de, // swords
      0xf554  // shoe
  };

  std::string iconHeart;
  std::string iconShield;
  std::string iconSwords;
  std::string iconShoe;
  // ICONS

  // PARTICLES
  std::vector<Particle> particles;
  // PARTICLES

  // LAYOUT
  float sw;
  float sh;

  Rectangle topBarRect;
  Rectangle listPanelRect;
  Rectangle infoPanelRect;
  std::vector<Rectangle> cardRects;

  Vector2 titlePos;
  float footerLineY;
  // LAYOUT

  // FONT SIZE
  float titleFontSize = 0.0f;
  float vsFontSize = 0.0f;

  float playerLabelFontSize = 0.0f;
  float playerSubFontSize = 0.0f;

  float cardNameFontSize = 0.0f;

  float nameFontSize = 0.0f;
  float roleFontSize = 0.0f;
  float descFontSize = 0.0f;

  float abilityTitleFontSize = 0.0f;
  float abilityDescFontSize = 0.0f;

  float statIconFontSize = 0.0f;
  float footerFontSize = 20.0f;
  // FONT SIZE

  // FUNCS
  std::vector<std::string> wrapTextToWidth(
      const std::string &, Font, float, float);

  void UpdateLayout();

  void handleKeyboard();
  void handleMouse();

  void initParticles();
  void updateParticles();
  void drawParticles();

  void drawBackground();
  void drawTitle();
  void drawTopBar();
  void drawHeroList();
  void drawInfoHero();
  void drawFooter();

  void drawStatBar(float x,
                   float y,
                   float width,
                   const char *icon,
                   int value,
                   int max,
                   Color color);
  // FUNCS

public:
  HeroSelectionScene(AudioManager *,
                     SceneManager *,
                     TextureManager *,
                     FontManager *);

  ~HeroSelectionScene();

  void Update() override;
  void Draw() override;
  void onEnter() override;
};