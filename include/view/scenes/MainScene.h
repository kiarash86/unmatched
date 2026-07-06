#pragma once                   // once per prg
#include "view/scenes/Scene.h" // father
#include "view/ui/Button.h"    // ui button
#include "view/ui/Particle.h"  // ui particle
#include <algorithm>           // min
#include <sstream>             // word-wrap

class MainScene : public Scene // main menu of game
{
private:
  struct quote // a struct for better readibility
  {
    std::string author; // author
    std::string text;   // text
  };
  // FONTS
  Font cinzelBold;     // title font
  Font cinzelSemiBold; // title button font
  Font cormoMedium;    // subtitle font
  Font cormoRegular;   // subtitle button font

  // TEXTURES
  Texture2D background; // texture
  Texture2D quoteBg;    // texture
  Texture2D logoTitle;  // texture
  Texture2D iconSword;  // texture
  Texture2D iconExit;   // texture
  Texture2D iconShield; // texture
  Texture2D iconBook;   // texture
  Texture2D iconCards;  // texture
  Texture2D clickedButton;  // texture
  Texture2D restButton;  // texture


  std::vector<Button> buttons;     // saving buttons here
  std::vector<Particle> particles; // particles are here
  std::vector<quote> quotes;       // saving quotes here

  int selected{0}; // which button is hovered?
  // LAYOUT
  float sw; // screen width
  float sh; // screnn height
  // LAYOUT

  int currentQuote{0}; // which quote?

  // ANIMATIONS
  float quoteOffset = 0.0f;       // moving quote
  float quoteTargetOffset = 0.0f; // moving quote
  // ANIMATIONS

  // TIME
  float timer{0.0f};
  // TIME

  // FONT SIZE
  float titleFontSize = 0.0f;    // font of title
  float subtitleFontSize = 0.0f; // font of subtitle

  float quoteFontSize = 0.0f;  // font of quote
  float authorFontSize = 0.0f; // font of author of quote
  float footerFontSize = 20.0f;

  // FONT SIZE

  // POSITIONS
  Rectangle logoRect;       // positon of logo
  Rectangle quotePanelRect; // position of quoteBox
  Vector2 titlePos;         // position of title
  Vector2 subtitlePos;      // position of subtitle
  float footerLineY;
  // POSITIONS

  // FUNCS
  std::vector<std::string> wrapTextToWidth(const std::string &, Font, float,
                                           float);

  void UpdateLayout(); // renew positions of ui componants

  void initParticles();   // creating particles
  void updateParticles(); // updating positons and... of particles
  void drawParticles();   // drawing particles

  void drawQuote();      // drawing a floating box with a text in it
  void drawBackground(); // drawing background
  void drawDarkLayer();  // a gradiant black for better visualition
  void drawButtons();    // drawing buttons(logo, text, subtext)
  void drawFooter();     // drawing footer
  void drawLogoTitle();  // drawing title, logo on top of title, subtitle

  void handleMouse();    // navigate with mouse(enter)
  void handleKeyboard(); // navigate with keyboard(enter)

  void activateButton(int); // which scene should we go?
  // FUNCS

public:
  // FUNCS
  MainScene(AudioManager *, SceneManager *, TextureManager *,
            FontManager *); // constructor
  void Update() override;   // update
  void Draw() override;     // drawing
  void onEnter() override;  // entering scene
  // don't need to do something when exiting
  // FUNCS
};