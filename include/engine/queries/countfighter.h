#include "engine/conditions/condition.h"
#include "engine/gameData.h"
#include "model/typeOfFighter.h"
#include "query.h"
class CountFighter : public Query {
private:
  TypeOfFighter fighterType;

public:
  CountFighter(TypeOfFighter type) : fighterType(type){};
  int get(gameData gameData) override {
    int value{0};
    for (auto &&fghr : gameData.map->getFighter(fighterType , gameData.self)) {
      bool flag = true;
      for (auto &&cnd : conditions) {
        if (!cnd->check(gameData, fghr)) {
          flag = false;
          break;
        }
      }
      if (flag) {

        value++; 
      }
    }
    return value;
  }
};
