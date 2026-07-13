#pragma once
#include "effect.h"

class RemoveCardEffect : public Effect
{
private:
    int howMany{1};
public:
    RemoveCardEffect(int howMany = 1) : howMany(howMany) {}
    ~RemoveCardEffect() = default;

    void execute(gameData & gameData) override
    {
        for (auto &&cond : conditions)
        {
            if (!cond->check(gameData))
            {
                return;
            }
        }
        int finalHowMany = howMany;
        for (auto &&query : queries)
        {
            finalHowMany += query->get(gameData);
        }


       // gameData.cardsRemovedThisTurn += finalHowMany;

    }
};
