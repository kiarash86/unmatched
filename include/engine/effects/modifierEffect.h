#include "effect.h"
class ModifierEffect : public Effect
{
private:
    int value{};
public:
    ModifierEffect(int value) : value(value){};
    ~ModifierEffect();
     
    
    void execute(gameData & gameData) override
     {
        int finalValue = value;
        for (auto &&cond : conditions)
        {
            if (!cond->check(gameData))
            {
                return;
            }
            
        }
        for (auto &&query : queries)
        {

            finalValue+= query->get(gameData);
        }
        
        gameData.cardPlayed->modifyValue(finalValue);


     }



};

