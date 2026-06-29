#include "model/card.h"
#include "model/fighter.h"
#include "engine/events/event.h"
class HealthChangeEvent : public Event {
Fighter * fighter; 
HealthChangeEvent( Fighter  * f)  : fighter (f){};



};