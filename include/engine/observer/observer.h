 #include "engine/events/event.h"
 class Observer
 {
 private:
    
 public:

    ~Observer() = default ;
    virtual void onEvent(Event &event) = 0;
    //FIXME : this is int because we dont have Event type yet

 };
 
