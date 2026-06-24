#include<vector>
#include "../gameData.h"
#include "../conditions/condition.h"
#include "engine/queries/query.h"
class Effect
{
protected:
    std::vector<std::unique_ptr<Condition>> conditions;
    std::vector<std::unique_ptr<Query>> queries;

public:
    Effect();
    void addCondition(std::unique_ptr<Condition> cond)
    {
        conditions.push_back(cond);
    }
   virtual ~Effect() = default;
   virtual void execute(gameData& gameData) = 0;
};

