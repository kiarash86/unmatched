#include "engine/observer/observer.h"
#include "engine/events/event.h"
#include <memory>
#include <vector>
class eventBus
{
private:
    std::vector<std::unique_ptr<Observer>> observers;
public:

    void addObserver(std::unique_ptr<Observer> ob){ observers.push_back(ob);}
    void emit(Event & ev)  {
        for (auto &ob : observers)
        {
            ob.onEvent(ev);
        }
        
    }
};
