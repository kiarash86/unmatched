#include "../gameData.h"
class Condition
{
private:
    
public:
    Condition();
   virtual ~Condition() = default;
   void check(gameData & gameData);
};


