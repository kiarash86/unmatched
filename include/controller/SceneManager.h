#pragma once // only once for entire prj
#include "controller/AudioManager.h"
#include "view/enums/ScenesType.h"            // for changing scenes
#include "view/scenes/HeroesSelectionScene.h" // selection scene
#include "view/scenes/MainScene.h"            // start scene
#include "view/scenes/Scene.h"                // scene
#include <memory>                             // smart pointer
class SceneManager {
private:
  // int screenHeight;
  // int screenWidth;
  AudioManager *audioManager;
  SceneManager *sceneManager;
  TextureManager *textureManager;
  FontManager *fontManager;
  // current scene that is being shown
  std::unique_ptr<Scene> currentScene;
  // current scene that is being shown

public:
  SceneManager(AudioManager *audioManager, TextureManager *textureManager,
               FontManager *fontManager)
      : audioManager(audioManager), textureManager(textureManager),
        fontManager(fontManager){};
  void changeScene(const ScenesType &type) {
    // first time(start menu) it wont do anything
    // but for next changing this will do things
    // before closing scene
    if (currentScene != nullptr) {
      currentScene->onExit();
    }
    // first time(start menu) it wont do anything
    // but for next changing this will do things
    // before closing scene

    // switch for deciding where to go
    switch (type) {
    case ScenesType::mainScene:
      currentScene = std::make_unique<MainScene>(audioManager, this,
                                                 textureManager, fontManager);
      break;
      // case ScenesType::collection:
      // TODO:: WRITE THESE SCENES , I DONT HAVE THEM FOR NOW
      //    break;
      //  case ScenesType::setting:
      // TODO:: WRITE THESE SCENES , I DONT HAVE THEM FOR NOW
      //    break;
    case ScenesType::heroSelection:
      currentScene = std::make_unique<HeroSelectionMenu>(
          audioManager, this, textureManager, fontManager);
      break;
    }
    // switch for deciding where to go

    // first things should be done after enetering new scenes
    currentScene->onEnter();
    // first things should be done after enetering new scenes
  };

  // same with scene, just for capsoulation funcs of scenes
  void draw() { currentScene->Draw(); };
  // same with scene, just for capsoulation funcs of scenes

  // same with scene, just for capsoulation funcs of scenes
  void update() { currentScene->Update(); };
  // same with scene, just for capsoulation funcs of scenes

  // destructor
  ~SceneManager() = default;
  // destructor
};
