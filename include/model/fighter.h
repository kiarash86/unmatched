#pragma once
#include "typeOfAttack.h"
#include "ability.h"
#include "sidekick.h"
#include "deck.h"
#include <memory>
#include <vector>
#include<string>
class Deck;
class Sidekick;
class Ability;

class Fighter
{
protected:
    std::string imgSource;
    std::string name;
    int health{};
    int maxHealth{};
    int movement{};
    TypeOfAttack typeOfAttack;

    bool canBeHealed() const;

public:
    void setHealth(const int&);
    int getHealth() const;    
    void setName(const std::string&);
    std::string getName() const;
    void setImgSource(const std::string&);
    std::string getImgSource() const;
    void setMovement(const int&);
    int getMovement()const;
    TypeOfAttack getTypeOfAttack() const;
    void setTypeOfAttack(const TypeOfAttack&);
    const std::vector<std::unique_ptr<Sidekick>>& getSidekicks() const;
    void addSidekick(std::unique_ptr<Sidekick>& sidekick);
        Ability * getAbility() const;
    Deck * getDeck() const;
    void damage(const int&);
    void heal(const int&);
    bool isAlive() const;



    fighter();
    ~fighter()= default;
};


//TODO : adding cpp of these funcs