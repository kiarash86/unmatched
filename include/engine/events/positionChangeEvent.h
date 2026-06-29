#include "model/card.h"
#include "model/fighter.h"
#include "engine/events/event.h"
class PositionChangeEvent : public Event {
Fighter * fighter; 
PositionChangeEvent( Fighter  * f)  : fighter (f){};

};