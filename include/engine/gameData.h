#include "../model/hero.h"
#include "../model/card.h"

struct gameData
{
    int board;
    int age;
    int target;
    int self;
    int enemy;
    Card * cardPlayed;
    int value;
};
