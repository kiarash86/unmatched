#pragma once
#include "typeOfAttack.h"
#include "ability.h"
#include "sidekick.h"
#include "deck.h"
#include <memory>
#include <vector>
#include<string>
#include "fighter.h"
class Deck;
class Sidekick;
class Ability;

class Hero : public Fighter
{
private:
   std::vector< std::unique_ptr<Sidekick>> sidekicks;
    std::unique_ptr<Deck> deck;
    std::unique_ptr<Ability> ability;


public:

    const std::vector<std::unique_ptr<Sidekick>>& getSidekicks() const;
    void addSidekick(std::unique_ptr<Sidekick>& sidekick);
        Ability * getAbility() const;
    Deck * getDeck() const;




    Hero();
    ~Hero()= default;
};


//TODO
// DELETE THINGS THAT ARE ALREADY IN FIGHTER