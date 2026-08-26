#pragma once
#include "raylib.h"
#include "controller/AudioManager.h"
#include "controller/FontManager.h"
#include "controller/SceneManager.h"
#include "controller/TextureManager.h"
#include "view/enums/ScenesType.h"
#include <memory>
class app {
private:
  std::unique_ptr<SceneManager> sceneManager;
  std::unique_ptr<AudioManager> audioManager;
  std::unique_ptr<FontManager> fontManager;
  std::unique_ptr<TextureManager> textureManager;

public:
  void run();
  app();
  ~app();
};

inline app::app() {
SetConfigFlags(
    FLAG_WINDOW_RESIZABLE |
    FLAG_WINDOW_UNDECORATED);  InitWindow(1980, 1080, "UNMATCHED");

SetWindowPosition(0, 0);

SetWindowSize(
    GetMonitorWidth(GetCurrentMonitor()),
    GetMonitorHeight(GetCurrentMonitor()));
  InitAudioDevice();
  SetTargetFPS(60);
  audioManager = std::make_unique<AudioManager>();
  fontManager = std::make_unique<FontManager>();
  textureManager = std::make_unique<TextureManager>();
  sceneManager = std::make_unique<SceneManager>(audioManager.get() ,textureManager.get() , fontManager.get());
  sceneManager->changeScene(ScenesType::mainScene);
}

inline app::~app() {
  sceneManager.reset();
  audioManager.reset();
  fontManager.reset();
  textureManager.reset();
  CloseAudioDevice();
  CloseWindow();
}

inline void app::run() {
  while (!WindowShouldClose()) {
    sceneManager->update();
    if (sceneManager->shouldQuit()) {
      break; 
    }
    audioManager->update();
    BeginDrawing();
    sceneManager->draw();
    EndDrawing();
  }
}