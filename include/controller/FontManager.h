#pragma once                        // only once for program
#include "libraries/magic_enum.hpp" //use name of enums
#include "raylib.h"                 // for font type
#include "view/enums/FontID.h"      // the key to fonts
#include <string>                   // string
#include <unordered_map>            // saving data in map (with key)

class FontManager // controll fonts
{
private:
  const std::string pathFonts = "assets/fonts/"; // where we save fonts
  std::unordered_map<std::string, Font> fonts;   // map of fonts(string key)

 
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
      UnloadFont(f.second);
    }
  }
//BUG:: awesome cant be used
  Font &getFont(FontID font, int size) // get font with font id and size
  {
    std::string baseName = std::string(magic_enum::enum_name(font));
    std::string name = baseName + "_" + std::to_string(size); // cache key

    auto it = fonts.find(name);
    if (it == fonts.end()) // check if we load this font before or no
    {
      if (baseName == "awesome") {
        int count = sizeof(awesomeCodepoints) / sizeof(awesomeCodepoints[0]);
        it = fonts
                 .emplace(name,
                          LoadFontEx((pathFonts + baseName + ".otf").c_str(),
                                     size,
                                     const_cast<int *>(awesomeCodepoints),
                                     count))
                 .first; // loading font with the actual icon codepoints

      } // FIXME:  for now using this exception for font awesome
      else {

        it = fonts
                 .emplace(name,
                          LoadFontEx((pathFonts + baseName + ".ttf").c_str(),
                                     size, nullptr, 0))
                 .first; // loading font, adding to map , get iterator of it
      }
    }   
    return it->second; // returning font
  }
};