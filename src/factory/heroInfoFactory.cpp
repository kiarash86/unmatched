#include "factory/heroInfoFactory.h"
std::unique_ptr<InfoHero> HeroInfoFactory::create(const HeroList &name) {
      std::string path ;   
    path = "data/" + (std::string)magic_enum::enum_name(name) + "/view.json" ;
    auto heroInfo = load(path);
    auto hero = std::make_unique<InfoHero>();
    hero->name = heroInfo["name"];
    hero->role = heroInfo["role"];
    hero->hp =(int) heroInfo["hp"];
    hero->defense =(int) heroInfo["defense"];

    hero->attack =(int) heroInfo["attack"];
    hero->difficulty =(int) heroInfo["difficulty"];

    hero->speed =(int) heroInfo["speed"];
    hero->desc = heroInfo["desc"];
    hero->abilityDesc = heroInfo["abilityDesc"];
    hero->abilityTitle = heroInfo["abilityTitle"];
    auto clr= heroInfo["themeColor"];
    hero->themeColor = Color{ 
        (unsigned char)clr[0] ,
        (unsigned char)clr[1] ,
        (unsigned char)clr[2] ,
        255

    } ;




    return hero;


}
