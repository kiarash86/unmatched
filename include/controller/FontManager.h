#pragma once                        // only once for program
#include "libraries/magic_enum.hpp" //use name of enums
#include "raylib.h"                 // for font type
#include "view/enums/FontID.h"      // the key to fonts
#include <filesystem>               // checking the font file exists
#include <string>                   // string
#include <unordered_map>            // saving data in map (with key)
#include <unordered_set>            // tracking which fonts are fallbacks

class FontManager // controll fonts
{
private:
  const std::string pathFonts = "assets/fonts/"; // where we save fonts
  std::unordered_map<std::string, Font> fonts;   // map of fonts(string key)
  std::unordered_set<std::string> isFallback; // fonts we shouldnt unload
                                             

  static constexpr int awesomeCodepoints[] = {
      0xf004, // heart
      0xf6de, // sword
      0xf132, // shield
      0xf554, // move
      0xf005  // star
  };

public:
  ~FontManager() // unloading fonts
  {
    for (auto &f : fonts) {
      if (isFallback.count(f.first)) continue;
      UnloadFont(f.second);
    }
  }

  Font &getFont(FontID font, int size) // get font with font id and size
  {
    std::string baseName = std::string(magic_enum::enum_name(font));
    std::string name = baseName + "_" + std::to_string(size); // cache key

    auto it = fonts.find(name);
    if (it == fonts.end()) // check if we load this font before or no
    {
      bool isAwesome = (baseName == "awesome");
      std::string filePath = pathFonts + baseName + (isAwesome ? ".otf" : ".ttf");

      if (!std::filesystem::exists(filePath)) {
        // No font asset shipped yet -- fall back to raylib's built-in
        // default font rather than handing raylib a bad path.
        it = fonts.emplace(name, GetFontDefault()).first;
        isFallback.insert(name);
      } else if (isAwesome) {
        int count = sizeof(awesomeCodepoints) / sizeof(awesomeCodepoints[0]);
        it = fonts
                 .emplace(name, LoadFontEx(filePath.c_str(), size,
                                          const_cast<int *>(awesomeCodepoints),
                                          count))
                 .first;
      } else {
        it = fonts.emplace(name, LoadFontEx(filePath.c_str(), size, nullptr, 0))
                 .first;
      }
    }
    return it->second; // returning font
  }
};
