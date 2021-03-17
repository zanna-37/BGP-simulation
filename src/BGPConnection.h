#ifndef BGPCONNECTION_H
#define BGPCONNECTION_H


#include <cassert>

#include "fsm/BGPStateMachine.h"
#include "fsm/BGPStateIdle.h"
#include "Event.h"

class BGPConnection {

public:

private:
    BGPStateMachine* stateMachine;
    //other BGPConnection variables

public:
    //Constructors
    BGPConnection();

    //Destructor
    ~BGPConnection();

    bool HandleEvent(const Event event);

    //Getter & Setter
    BGPStateMachine* getStateMahine(){return stateMachine;}

};

#endif