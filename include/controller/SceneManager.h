#pragma once // only once for entire prj
#include "controller/AudioManager.h"
#include "utility/exceptions.h"               // for errors
#include "view/enums/ScenesType.h"            // for changing scenes
#include "view/scenes/GameScene.h"            // board scene
#include "view/scenes/HeroSelectionScene.h" // selection scene
#include "view/scenes/LoadGameScene.h"       // load-game scene
#include "view/scenes/MainScene.h"            // start scene
#include "view/scenes/Scene.h"                // scene
#include <iostream>                           // for logging recovered errors
#include <memory>                             // smart pointer
class SceneManager {
private:
  AudioManager *audioManager;
  TextureManager *textureManager;
  FontManager *fontManager;
  // current scene that is being shown
  std::unique_ptr<Scene> currentScene;

  bool quitRequested{false};

  // current scene that is being shown

  // this func is created for using that in change scene 
  //so we can find error with more controll over it
  void buildScene(const ScenesType &type) {
    switch (type) {
    case ScenesType::mainScene:
      currentScene = std::make_unique<MainScene>(audioManager, this,
                                                 textureManager, fontManager);
      break;
      case ScenesType::game:
      currentScene = std::make_unique<GameScene>(audioManager , this , textureManager , fontManager);
      break;
    case ScenesType::heroSelection:
      currentScene = std::make_unique<HeroSelectionScene>(
          audioManager, this, textureManager, fontManager);
      break;
    case ScenesType::loadGame:
      currentScene = std::make_unique<LoadGameScene>(audioManager, this,
                                                      textureManager, fontManager);
      break;
    default:
    throw AppException("no scene");
    }
  }

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

    // using try catch when we are moving to a new scene
    try {
      buildScene(type);
    } catch (const AppException &e) {
      std::cerr << "[SceneManager] Failed to enter scene, returning to main menu: "
                << e.what() << std::endl;
      if (type == ScenesType::mainScene) {
  
        throw;
      }
      buildScene(ScenesType::mainScene); // error? go back to menu
    }

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

  void requestQuit() { quitRequested = true; };
  bool shouldQuit() const { return quitRequested; };

  // destructor
  ~SceneManager() = default;
  // destructor
};
