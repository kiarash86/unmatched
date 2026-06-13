#include "effect.h"
class MoveEffect : public Effect
{
private:
    
public:
    MoveEffect();
    ~MoveEffect();
     void excute(gameData& gameData) override;
};

