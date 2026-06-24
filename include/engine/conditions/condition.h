#include "../gameData.h"
class Condition
{
private:
    
public:
    Condition();
   virtual ~Condition() = default;
   virtual bool check( gameData & gameData, Fighter* fighter = nullptr )=0;
};


