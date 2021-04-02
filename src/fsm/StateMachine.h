#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <cassert>
#include <condition_variable>
#include <queue>
#include <thread>

#include "../entities/Device.h"
#include "../logger/Logger.h"
template <class Connection, class State, class Event>
class StateMachine {
   private:
    State* previousState = nullptr;
    State* currentState  = nullptr;

    std::thread*            eventHandler = nullptr;
    std::mutex              eventQueue_mutex;
    std::condition_variable eventQueue_ready;
    queue<Event>            eventQueue;

   public:
    Connection* connection = nullptr;

    StateMachine(Connection* connection);

    ~StateMachine();

    void changeState(State* newState);

    void enqueueEvent(Event);

    State* getCurrentState() const;
};
#endif