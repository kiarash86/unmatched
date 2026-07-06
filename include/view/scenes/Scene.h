#pragma once                           // once per prg
#include "controller/AudioManager.h"   // audio manager
#include "controller/FontManager.h"    // font manager
#include "controller/TextureManager.h" // texture manager
#include "raylib.h"                    // graphic with ?
#include "view/enums/FontID.h"         // list of fonts
#include "view/enums/MusicID.h"        // list of music
#include "view/enums/ScenesType.h"     // list of scenes
#include "view/enums/SoundID.h"        // list of sounds
#include "view/enums/TextureID.h"      // list of textures
#include <string>                      // string
#include <vector>                      // vector

class SceneManager; // because of circular including

// no cpp file
class Scene // father of all scenes
{
protected:
  AudioManager *audio;     // pointer to audio manager
  SceneManager *scene;     // pointer to scene manager
  TextureManager *texture; // pointer to texture manager
  FontManager *font;       // pointer to font manager

public:
  Scene(AudioManager *audioManager, SceneManager *sceneManager,
        TextureManager *textureManager, FontManager *fontManager)
      : audio(audioManager), scene(sceneManager), texture(textureManager),
        font(fontManager){};

  virtual void Draw() = 0;    // drawing
  virtual void Update() = 0;  // update
  virtual void onExit() {};   // when exiting scene
  virtual void onEnter() {};  // when entering scene
  virtual ~Scene() = default; // distructor
};