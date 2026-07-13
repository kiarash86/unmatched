#pragma once
#include "effect.h"

class RemoveEffectEffect : public Effect
{
private:
    int howMany{-1}; // -1 for "all"
public:
    RemoveEffectEffect(int howMany = -1) : howMany(howMany) {}
    ~RemoveEffectEffect() = default;

    void execute(gameData & gameData) override
    {
        for (auto &&cond : conditions)
        {
            if (!cond->check(gameData))
            {
                return;
            }
        }


    }
};
