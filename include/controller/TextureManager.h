#pragma once                        // only once for program
#include "libraries/magic_enum.hpp" //use name of enums
#include "raylib.h"                 // for texture type
#include "view/enums/TextureID.h"   // the key to textures
#include <filesystem>               // cheking folders
#include <string>                   // string
#include <unordered_map>            // saving data in map

class TextureManager // controlling textures
{
private:
  std::unordered_map<TextureID, Texture2D> textures; // saving textures here

  const std::string texturePath = "assets/images/"; // path of textures

public:
  Texture2D &getTexture(TextureID texture) // get texture
  {
    return textures.at(texture); // return texture
  }

  void loadAllTextures() // loading all images
  {
    for (auto &entry : std::filesystem::recursive_directory_iterator(
             texturePath)) // checking every folder in images
    {
      if (!entry.is_regular_file())
        continue;

      auto stem = entry.path().stem().string();

      auto id = magic_enum::enum_cast<TextureID>(stem);

      if (id.has_value()) {
        textures.emplace(*id, LoadTexture(entry.path().string().c_str()));
      }
    }
  }

  TextureManager() // loading textures
  {
    loadAllTextures();
  }

  ~TextureManager() // unloading textures
  {
    for (auto &[id, tex] : textures) {
      UnloadTexture(tex);
    }
  }
};