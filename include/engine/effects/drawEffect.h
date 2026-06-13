#include "effect.h"
class DrawEffect : public Effect
{
private:
    
public:
    DrawEffect();
    ~DrawEffect();
     void excute(gameData& gameData) override;
};

