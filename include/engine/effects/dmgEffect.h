#include "effect.h"
class DmgEffect : public Effect
{
private:
    
public:
    DmgEffect();
    ~DmgEffect();
     void excute(gameData & gameData) override
     {
        gameData.target->dmg(gameData.cardPlayed.value);
     }
};

