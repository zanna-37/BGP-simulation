#ifndef STATE_H
#define STATE_H
#include <stack>

#include "../../logger/Logger.h"
#include "StateMachine.h"


template <class Protocol>
class StateMachine;

template <class Protocol>
class State {
   public:
    StateMachine<Protocol>* stateMachine;
    std::string             NAME;

    State(StateMachine<Protocol>* stateMachine) : stateMachine(stateMachine) {}
    ~State() {}

    virtual bool onEvent(Event<Protocol>) = 0;
};
#endif