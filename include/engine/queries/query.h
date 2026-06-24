#include "engine/gameData.h"
#include "engine/conditions/condition.h"
#include "model/typeOfFighter.h"
class Query
{
    protected:
      std::vector<std::unique_ptr<Condition>> conditions;

private:
    
public:
  void addCondition(std::unique_ptr<Condition> cond){ conditions.push_back(std::move(cond));}
 virtual int get(gameData gameData)= 0 ;
 ~Query() = default;
};