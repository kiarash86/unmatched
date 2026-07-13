#pragma once
#include "effect.h"

class PositionExchangeEffect : public Effect
{
private:
public:
    PositionExchangeEffect() {}
    ~PositionExchangeEffect() = default;

    void execute(gameData & gameData) override
    {
        for (auto &&cond : conditions)
        {
            if (!cond->check(gameData))
            {
                return;
            }
        }
        //something like this
        //gameData.board->exchangePosition(gameData.target);


    }
};
