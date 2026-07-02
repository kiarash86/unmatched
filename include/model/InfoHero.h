#pragma once
#include <string>
#include <raylib.h>
struct InfoHero
{

    std::string wallpaperPath;
    std::string logoPath;

    std::string name; 
    std::string role;
    std::string desc;
    int hp;
    int attack;
    int defense;
    int speed;
    std::string abilityTitle;
    std::string abilityDesc;
    Color themeColor;
    Texture2D wallpaper;
    Texture2D logo;

};
