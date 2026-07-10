#pragma once
#include "factory/heroInfoFactory.h"
#include "libraries/IconsFontAwesome6.h"
#include "model/InfoHero.h"
#include "model/heroList.h"
#include "raylib.h"
#include "view/scenes/Scene.h"
#include <memory>
#include <string>
#include <vector>
struct Player {
  int selectedHeroIndex;
  bool isReady;
};

class HeroSelectionMenu : public Scene {
private:
  Font iconFont;
  std::vector<std::unique_ptr<InfoHero>> heroes;
  Player players[2];
  int currentPlayer;
  int selected;
  Color bgDark;
  Color panelBg;
  Color gold;
  Color textLight;
  Color textMuted;
  Texture2D background;

  void DrawStatBar(float x, float y, int value, int max, Color color) {
    int barWidth = 5;
    DrawLineEx({x, y}, {x + max * 14, y}, barWidth, DARKGRAY);
    DrawLineEx({x, y}, {x + value * 14, y}, barWidth, color);
  }

  int codepoints[4] = {
      0xf004, // Heart
      0xf3ed, // Shield Halved
      0xe03d, // Swords
      0xf54b  // Shoe Prints
  };

public:
  HeroSelectionMenu(AudioManager * audioManager ,  SceneManager * sceneManager,  TextureManager * textureManager,  FontManager * fontManager)  : Scene(audioManager , sceneManager ,textureManager , fontManager) {

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    iconFont = LoadFontEx("assets/fontawesome.otf", 64, codepoints, 4);

    // HACK:  we will use factory here but its not correct and it should go to
    // gameManager

    heroes.push_back(HeroInfoFactory::create(HeroList::Dracula));
    heroes.push_back(HeroInfoFactory::create(HeroList::SherlockHolms));

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

    bgDark = {15, 15, 15, 255};
    panelBg = {25, 25, 25, 240};
    gold = {212, 175, 55, 255};
    textLight = {220, 220, 220, 255};
    textMuted = {150, 150, 150, 255};
  }

  bool IsDone() const { return players[0].isReady && players[1].isReady; }

  void Update() {
    if (IsKeyPressed(KEY_DOWN)) {
      selected = (selected + 1) % heroes.size();
    }
    if (IsKeyPressed(KEY_UP)) {
      selected = (selected - 1 + heroes.size()) % heroes.size();
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

  void Draw() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    ClearBackground(bgDark);

    auto currentHero = heroes[selected].get();

    DrawRectangleGradientV(0, 0, screenWidth, screenHeight,
                           currentHero->themeColor, bgDark);

    DrawTexturePro(currentHero->wallpaper,
                   {0, 0, (float)currentHero->wallpaper.width,
                    (float)currentHero->wallpaper.height},
                   {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                   {0, 0}, 0, WHITE);

    int marginLeft = 20;
    int marginTop = 170;
    int cardHeight = 200;
    int cardWidth = 180;
    for (int i = 0; i < (int)heroes.size(); i++) {
      int cardY = marginTop + (i * cardHeight);

      // a rectangle that is a container for logo and name of the hero
      Rectangle cardRect = {(float)marginLeft, (float)cardY, (float)cardWidth,
                            (float)cardHeight};
      // a rectangle that is a container for logo and name of the hero

      bool isHovered = (i == selected);

      Color backGroundColor;
      if (isHovered) {
        backGroundColor = Color{40, 45, 60, 220};
      } else {
        backGroundColor = Color{15, 15, 15, 170};
      }
      DrawRectangleRoundedLines(cardRect, 0.15f, 8, 2, backGroundColor);

      //  DrawRectangleRec(cardRect , BLACK);

      DrawText(heroes[i]->name.c_str(), marginLeft, cardY + 100, 24,
               isHovered ? WHITE : textMuted);
    }

    int rightPanelX = 850;
    DrawText(currentHero->name.c_str(), rightPanelX, 200, 50, WHITE);
    DrawText(currentHero->role.c_str(), rightPanelX, 260, 20, SKYBLUE);
    DrawText(currentHero->desc.c_str(), rightPanelX, 295, 18, textLight);

    int statY = 400;
    DrawText("Health", rightPanelX, statY, 18, WHITE);
    DrawStatBar(rightPanelX + 80, statY, currentHero->hp, 10, RED);

    DrawText("Attack", rightPanelX, statY + 30, 18, WHITE);
    DrawStatBar(rightPanelX + 80, statY + 30, currentHero->attack, 10, ORANGE);

    DrawText("Defense", rightPanelX, statY + 60, 18, WHITE);
    DrawStatBar(rightPanelX + 80, statY + 60, currentHero->defense, 10, gold);

    DrawText("Speed", rightPanelX, statY + 90, 18, WHITE);
    DrawStatBar(rightPanelX + 80, statY + 90, currentHero->speed, 10, BLUE);

    DrawText("Ability", rightPanelX, statY + 150, 18, gold);
    DrawText(currentHero->abilityTitle.c_str(), rightPanelX, statY + 175, 22,
             WHITE);
    DrawText(currentHero->abilityDesc.c_str(), rightPanelX, statY + 205, 18,
             textMuted);

    int fixedTopBarHeight = 100;
    Rectangle topBarRect = {20, 20, (float)screenWidth - 40,
                            (float)fixedTopBarHeight};
    DrawRectangleRounded(topBarRect, 0.1f, 10, panelBg);
    DrawRectangleRoundedLines(topBarRect, 0.1f, 10, 2, gold);

    DrawText("VS", screenWidth / 2 - 20, 20 + (fixedTopBarHeight / 2) - 20, 40,
             gold);

    for (int p = 0; p < 2; p++) {
      int pX = (p == 0) ? 50 : screenWidth / 2 + 100;
      int pY = 40;

      if (players[p].isReady) {
        auto pickedHero = heroes[players[p].selectedHeroIndex].get();
        DrawTexturePro(
            heroes[players[p].selectedHeroIndex]->logo,
            {0, 0, (float)heroes[players[p].selectedHeroIndex]->logo.width,
             (float)heroes[players[p].selectedHeroIndex]->logo.height},
            {(float)(pX), (float)(pY - 10), (float)80, (float)80}, {0, 0}, 0,
            WHITE);

        //   DrawRectangle(pX, pY, 60, 60, pickedHero->themeColor);
        //  DrawRectangleLines(pX, pY, 60, 60, gold);
        DrawText(pickedHero->name.c_str(), pX + 80, pY, 24, WHITE);
        DrawText(pickedHero->role.c_str(), pX + 80, pY + 25, 16, textMuted);
        DrawText("Ready", pX + 80, pY + 45, 16, SKYBLUE);
      } else {
        DrawRectangle(pX, pY, 60, 60, DARKGRAY);
        if (currentPlayer == p) {
          DrawText("?", pX + 22, pY + 15, 30, textLight);
          DrawText(p == 0 ? "Player 1" : "Player 2", pX + 80, pY + 10, 20,
                   (p == 0) ? SKYBLUE : RED);
          DrawText("Choosing...", pX + 80, pY + 35, 16, WHITE);
        } else {
          DrawText(p == 0 ? "Player 1" : "Player 2", pX + 80, pY + 10, 20,
                   DARKGRAY);
          DrawText("Waiting...", pX + 80, pY + 35, 16, DARKGRAY);
        }
      }
    }

    int bottomY = screenHeight - 50;
    DrawText("UP/DOWN: Navigate", screenWidth / 2 - 200, bottomY, 18,
             textMuted);
    DrawText("ENTER: Select", screenWidth / 2, bottomY, 18, textMuted);
    DrawText("ESC: Back", screenWidth / 2 + 200, bottomY, 18, textMuted);
  }

  ~HeroSelectionMenu() {
    for (auto &hero : heroes) {
      UnloadTexture(hero->wallpaper);
    }
  }
};
