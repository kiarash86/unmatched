#pragma once                        // only once for program
#include "libraries/magic_enum.hpp" //use name of enums
#include "raylib.h"                 // for texture type
#include "view/enums/TextureID.h"   // the key to textures
#include <cctype>                   // toupper/tolower
#include <filesystem>               // cheking folders
#include <optional>                 // optional return for card lookup
#include <string>                   // string
#include <unordered_map>            // saving data in map
#include <utility>                  // std::pair
#include <vector>                   // vector

class TextureManager // controlling textures
{
private:
  std::unordered_map<TextureID, Texture2D> textures; // saving textures here

  const std::string texturePath = "assets/images/"; // path of textures


  Texture2D makePlaceholder() // if couldnt load imgs we will use this instead of ruining scene
  {
    Image img = GenImageColor(64, 64, MAGENTA);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
  }

public:
  Texture2D &getTexture(TextureID texture) // get texture
  {
    auto it = textures.find(texture);
    if (it == textures.end()) // checking img is loaded or no, if not placeholder
    {
      it = textures.emplace(texture, makePlaceholder()).first;
    }
    return it->second;
  }

  void loadAllTextures() // loading all images
  {
    if (!std::filesystem::exists(texturePath)) {
      return; // nothing in path? then use placeholder
    }

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
