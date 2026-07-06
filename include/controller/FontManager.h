
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
  std::unordered_map<std::string, Font> fonts;  // map of fonts(string key)

public:
  ~FontManager() // unloading fonts
  {
    for (auto &f : fonts) {
      UnloadFont(f.second);
    }
  }

  Font &getFont(FontID font, int size) // get font with font id and size
  {
    std::string name; // creating font key with this
    name =
        std::string(magic_enum::enum_name(font)) + "_" + std::to_string(size);

    auto it = fonts.find(name);
    if (it == fonts.end()) // check if we load this font before or no
    {

      it = fonts
               .emplace(name,
                        LoadFontEx((pathFonts +
                                    std::string(magic_enum::enum_name(font)) +
                                    ".ttf")
                                       .c_str(),
                                   size, nullptr, 0))
               .first; // loading font, adding to map , get iterator of it
    }
    return it->second; // returning font
  }
};
