#pragma once                         // once per prg
#include "factory/heroInfoFactory.h" // factory for showing info
#include "model/InfoHero.h"          // model of info hero
#include "model/Player.h"            // saving heroes in players
#include "model/heroList.h"          // list of heroes
#include "view/scenes/Scene.h"       // father
#include <memory>                    // ptr
#include <sstream>                   //wrap texts
#include <vector>                    //vector

class HeroSelectionScene : public Scene // scene of selecting heroes
{
private:
enum class PendingVoiceAction {
none,
Player2,
startTransition
  }; /*this is for controlling
  sounds, if it is player2 then at first it should reach the end of sspeech of
  first hero then go  choose hero for player 2*/

PendingVoiceAction pendingVoiceAction =
PendingVoiceAction::none; //   where are we now?
SoundID heroVoiceBeingWaitedOn =
SoundID::draculaSpeech; // next sound 

std::vector<std::unique_ptr<InfoHero>> heroes; // saving in this vector

std::vector<HeroList> heroIds; // FIXME

int selectedHero = 0; // index for buttons

static const int totalPlayers = 2; // number of player we need

int currentPlayerIndex = 0; // which player turn it is

  // (1 player vs AI, or 2 player)
bool vsAIMode = false; // false = 2 players, true =ai
bool gameModeLocked =
    false;
  // GAME MODE

  // FONTS
Font cinzelBold;
Font cinzelSemiBold;
Font cormoMedium;
Font cormoRegular;
  // FONTS

  // FONT SIZE
float heroNameFontSize = 0;

float heroRoleFontSize = 0;

float panelTitleFontSize = 0;

float descriptionFontSize = 0;

float statFontSize = 0;

float buttonFontSize = 0;
  // FONT SIZE

  // LAYOUT
float sw = 0; // screen width
float sh = 0; // screen height

Rectangle backButtonRect; // return to main menu
Rectangle heroListRect;   // menu
Rectangle infoPanelRect;  // info box
Rectangle titleImageRect; // where to draw titleHeroSelection
Rectangle paginationRect; // where to draw hero pagination dots

Rectangle onePlayerModeRect; // 1 Player button
Rectangle twoPlayerModeRect; // 2 Players button

std::vector<Rectangle> heroCardRects; // rect of each hero card in the list
  // LAYOUT

  // FUNCS
void UpdateLayout(); // layout controller

void handleKeyboard(); // navigate and click with keyboard
void handleMouse();    // navigate and click with mouse

void confirmSelection();        // effect of selected for hero
void moveToNextAvailableHero(); // go to next hero that is not selected by
                                  // someone before

void drawBackground();  // draw background
void drawTitle();       // draw title
void drawBackButton();  // draw back button
void drawModeToggle();  // draw 1 player / 2 player toggle buttons
void drawHeroList();    // draw hero list(menu)
void drawInfoPanel();  // draw panel of info
void drawDifficultyStars(float x, float y, int rating,
                          int maxRating = 5); // draw difficulty stars

TextureID
bgForHero(HeroList id); // helper func for get id and return texture id

TextureID normalButtonForHero(
HeroList id); // helper func for get id and return texture()
TextureID hoveredButtonForHero(
HeroList id); // helper func for get id and return texture()

SoundID soundForHero(HeroList id); // get id and return speech sound

public:
HeroSelectionScene(AudioManager *, SceneManager *, TextureManager *,
FontManager *); // constructor
~HeroSelectionScene();             // destructor
void Update() override;            // update
void Draw() override;              // draw
void onEnter() override;           // onEnter
                                     // FUNCS
};