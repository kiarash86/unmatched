#pragma once
#include <string>
#include <raylib.h>
struct InfoHero
{

    std::string name; 
    std::string role;
    std::string desc;
    int hp;
    int attack;
    int defense;
    int speed;
    int difficulty;
    std::string abilityTitle;
    std::string abilityDesc;
    Color themeColor;
    Texture2D wallpaper;
    Texture2D normalButton;
    Texture2D hoveredButton;

};
