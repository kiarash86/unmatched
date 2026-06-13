#include "../gameData.h"
class Effect
{
private:
    
public:
    Effect();
   virtual ~Effect() = default;
   virtual void excute(gameData& gameData) = 0;
};

