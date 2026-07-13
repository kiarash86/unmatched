#pragma once
#include "effect.h"

class SeeHandEffect : public Effect
{
public:
    SeeHandEffect() = default;
    ~SeeHandEffect() = default;

    void execute(gameData & gameData) override
    {
        for (auto &&cond : conditions)
        {
            if (!cond->check(gameData))
            {
                return;
            }
        }

        // see hand

    }
};
