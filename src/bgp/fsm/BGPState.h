#ifndef BGPSTATE_H
#define BGPSTATE_H

#include "BGPStateMachine.h"
#include "../Event.h"
#include "../../logger/Logger.h"

class BGPStateMachine; //forward declaration

class BGPState { 

public:
    virtual void enter() = 0;
    virtual void execute() = 0;
    virtual void exit() = 0;
    BGPStateMachine* stateMachine;
    string NAME;
    BGPState(BGPStateMachine* stateMachine): stateMachine(stateMachine){};
    virtual ~BGPState(){}

    virtual bool onEvent(Event)=0;

    //You can implement here some global reactions to Events
};

#endif