#pragma once        // once per prg
#include "raylib.h" // rec type
#include <string>   // string

struct Button // a struct for button
{
  Rectangle rec;
  Texture2D icon;
  std::string title;
  std::string subTitle;
};