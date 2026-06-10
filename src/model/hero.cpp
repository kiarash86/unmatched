#include "../../include/model/hero.h"


    bool Hero::canBeHealed() const
    {
        return (isAlive() and health < maxHealth);
    }

    void Hero::setHealth(const int& newHealth)
    {
        health = newHealth;
    }
    int Hero::getHealth() const
    {
        return health;
    }
    void Hero::setMovement(const int &newMovement)
    {
        movement = newMovement;
    }
    int Hero::getMovement()const
    {
        return movement;
    }
    TypeOfAttack Hero::getTypeOfAttack() const
    {
        return typeOfAttack;
    }
    void Hero::setTypeOfAttack(const TypeOfAttack & newTypeOfAttack)
    {
        typeOfAttack = newTypeOfAttack;
    }
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
    void Hero::damage(const int &dmg)
    {

        health -=((health-dmg) <0) ? health : dmg;
    }
    void Hero::heal(const int &heal)
    {
        if (canBeHealed())
        {
            health +=((health+heal) > maxHealth) ? maxHealth-health : heal;
        }
        
    }
    bool Hero::isAlive() const
    {
        return health;
    }
    void Hero::setName(const std::string &newName)
    {
        name =newName;
    }
    std::string Hero::getName() const
    {
        return name;
    }
    void Hero::setImgSource(const std::string &newImgSource)
    {
        imgSource = newImgSource;
    }
    std::string Hero::getImgSource() const
    {
        return imgSource;
    }

    Hero::Hero() {}
 