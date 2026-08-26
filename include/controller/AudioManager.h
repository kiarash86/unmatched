#pragma once                        // once per prg
#include "libraries/magic_enum.hpp" // getting name of enums and...
#include "raylib.h"                 // sound and music type
#include "view/enums/MusicID.h"     // list of music
#include "view/enums/SoundID.h"     // list of sounds
#include <filesystem>               // checking files exist before loading
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
    if (!std::filesystem::exists(pathSounds))
      return; // no assets yet

    for (auto &snd :
         magic_enum::enum_values<SoundID>()) // soundID is list of all sounds
    {
      std::string path = pathSounds + std::string(magic_enum::enum_name(snd)) + ".mp3";
      if (!std::filesystem::exists(path))
        continue; // this one's missing
      sounds.emplace(snd, LoadSound(path.c_str()));
    }
  }

  void loadAllMusic() // loading all musics
  {
    if (!std::filesystem::exists(pathMusics))
      return; // no assets yet

    for (auto &msc :
         magic_enum::enum_values<MusicID>()) // musicID is list of all muaics
    {
      std::string path = pathMusics + std::string(magic_enum::enum_name(msc)) + ".mp3";
      if (!std::filesystem::exists(path))
        continue; // this one's missing
      musics.emplace(msc, LoadMusicStream(path.c_str()));
    }
  }

public:
  void update() // get music for seconds ahead
  {
    for (auto &cMs : currentMusics)
    {
      auto it = musics.find(cMs);
      if (it == musics.end())
        continue;
      if (IsMusicStreamPlaying(it->second)) // check if its puased
      {
        UpdateMusicStream(it->second);
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
    auto it = musics.find(music);
    if (it == musics.end())
    {
      return;
    }
    if (currentMusics.count(music) != 0)
    {
      SetMusicVolume(it->second, volume);
      it->second.looping = loop;
      return;
    }

    SetMusicVolume(it->second, volume); // set volume
    it->second.looping = loop;          // set looping status
    PlayMusicStream(it->second);        // playing music
    currentMusics.insert(music);        // adding to current musics
  }

  void playSound(SoundID sound, float volume) // play sound
  {
    auto it = sounds.find(sound);
    if (it == sounds.end())
      return;                           // asset missing, no-op
    SetSoundVolume(it->second, volume); // set volume
    PlaySound(it->second);              // playing sound
  }

  void stopAllMusic() // no music playing anymore
  {
    for (auto &ms : currentMusics)
    {
      auto it = musics.find(ms);
      if (it != musics.end())
        StopMusicStream(it->second);
    }
    currentMusics.clear();
  }

  void stopMusic(MusicID music) // erase this music from current musics
  {
    auto it = musics.find(music);
    if (it != musics.end())
      StopMusicStream(it->second);
    currentMusics.erase(music);
  }

  void pauseMusic(MusicID music) // pause musicStream
  {
    auto it = musics.find(music);
    if (it == musics.end())
      return;
    if (IsMusicStreamPlaying(it->second))
    {
      PauseMusicStream(it->second);
    }
  }

  void resumeMusic(MusicID music) // continue musicStream
  {
    auto it = musics.find(music);
    if (it == musics.end())
      return;
    if (!IsMusicStreamPlaying(it->second))
    {
      ResumeMusicStream(it->second);
    }
  }

  bool isSoundPlaying(SoundID sound) // is this sound still playing
  {
    auto it = sounds.find(sound);
    return it != sounds.end() && IsSoundPlaying(it->second);
  }
};
