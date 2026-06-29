#include "model/card.h"
#include "engine/events/event.h"
#include "model/hero.h"
class CardPlayedEvent : public Event {
    public:
    //FIXME:  no player model yet and probably it shouldnt work with player

    Player * player ;
    CardPlayedEvent( Player *  p) : player(p){};
};