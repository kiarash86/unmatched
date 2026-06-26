#include "../../include/model/hero.h"
const std::vector<std::unique_ptr<Sidekick>>& Hero::getSidekicks() const
    {
        return sidekicks;
        
    }
        Ability * Hero::getAbility() const
        {
            return ability.get();
        }
        Deck * Hero::getDeck() const
        {
            return deck.get();
        }
    Hero::Hero() {}
 

        void Hero::addSidekick(std::unique_ptr<Sidekick>& sidekick)
        {
            sidekicks.push_back(sidekick);
        }


