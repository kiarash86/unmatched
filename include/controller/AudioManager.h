#pragma once                        // once per prg
#include "libraries/magic_enum.hpp" // getting name of enums and...
#include "raylib.h"                 // sound and music type
#include "view/enums/MusicID.h"     // list of music
#include "view/enums/SoundID.h"     // list of sounds
#include <string>                   // string
#include <unordered_map>            // saving all musics and sounds here
#include <unordered_set>            // current musics that are being played

class AudioManager // controll musics and sounds
{
private:
  const std::string pathMusics = "assets/music/"; // path of music

  const std::string pathSounds = "assets/sound/"; // path of sound

  std::unordered_set<MusicID> currentMusics; // musics that are being played

  std::unordered_map<MusicID, Music> musics; // all musics

  std::unordered_map<SoundID, Sound> sounds; // all sounds

  void loadAllSound() // loading all sounds
  {
    for (auto &msc :
         magic_enum::enum_values<SoundID>()) // soundID is list of all sounds
    {
      sounds.emplace(
          msc, LoadSound((pathSounds + std::string(magic_enum::enum_name(msc)) +
                          ".mp3")
                             .c_str()));
    }
  }

  void loadAllMusic() // loading all musics
  {
    for (auto &msc :
         magic_enum::enum_values<MusicID>()) // musicID is list of all muaics
    {

      musics.emplace(msc, LoadMusicStream(
                              (pathMusics +
                               std::string(magic_enum::enum_name(msc)) + ".mp3")
                                  .c_str()));
    }
  }

public:
  void update() // get music for seconds ahead
  {

    for (auto &cMs : currentMusics) {
      if (IsMusicStreamPlaying(musics.at(cMs))) // check if its puased
      {

        UpdateMusicStream(musics.at(cMs));
      }
    }
  }

  AudioManager() // loading audioes
  {
    loadAllMusic();
    loadAllSound();
  }

  ~AudioManager() // unloading audioes
  {
    stopAllMusic();          // stop every music at first
    for (auto &msc : musics) // uloading one by one
    {
      UnloadMusicStream(msc.second);
    }
    for (auto &sd : sounds) // uloading one by one
    {
      UnloadSound(sd.second);
    }
  }

  void playMusic(MusicID music, float volume, bool loop) // play music
  {

    if (currentMusics.find(music) !=
        currentMusics.end()) // dont have this music? bye!bye!
    {
      return;
    }

    SetMusicVolume(musics.at(music), volume); // set volume
    musics.at(music).looping = loop;          // set looping status
    PlayMusicStream(musics.at(music));        // playing music
    currentMusics.insert(music);              // adding to current musics
  }

  void playSound(SoundID sound, float volume) // play sound
  {
    SetSoundVolume(sounds.at(sound), volume); // set volume
    PlaySound(sounds.at(sound));              // playing sound
  }

  void stopAllMusic() // no music playing anymore
  {
    for (auto &ms : currentMusics) {
      StopMusicStream(musics.at(ms));
    }
    currentMusics.clear();
  }

  void stopMusic(MusicID music) // erase this music from current musics
  {
    StopMusicStream(musics.at(music));
    currentMusics.erase(music);
  }

  void pauseMusic(MusicID music) // pause musicStream
  {
    if (IsMusicStreamPlaying(musics.at(music))) {
      PauseMusicStream(musics.at(music));
    }
  }

  void resumeMusic(MusicID music) // continue musicStream
  {
    if (!IsMusicStreamPlaying(musics.at(music))) {

      ResumeMusicStream(musics.at(music));
    }
  }
};