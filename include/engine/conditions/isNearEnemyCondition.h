#include "condition.h"
#include "model/fighter.h"
class IsNearEnemyCondition : public Condition {
private:

  int distance{};

public:
  IsNearEnemyCondition( int distance)
      :  distance(distance){};
  ~IsNearEnemyCondition();
  bool check(gameData &gameData, Fighter* fighter ) override {
    //check distance between this 2
    if (fighter->getPosition() == gameData.enemy->position)
    {
      //FIXME
      return true;
    }
    
    return false;
    
  }
};
