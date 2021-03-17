#ifndef BGPSTATE_H
#define BGPSTATE_H

#include "fsm/BGPStateMachine.h"
#include "Event.h"


class BGPState { 

public:
    virtual void enter() = 0;
    virtual void execute() = 0;
    virtual void exit() = 0;
    BGPStateMachine* stateMachine;
    BGPState(BGPStateMachine* stateMachine): stateMachine(stateMachine){};
    virtual ~BGPState(){}

    virtual bool OnEvent(Event)=0;

    //You can implement here some global reactions to Events
};

#endif