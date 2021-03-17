#ifndef BGPSTATEMACHINE_H
#define BGPSTATEMACHINE_H

#include <cassert>


#include "fsm/BGPState.h"
#include "BGPConnection.h"
#include "Event.h"
#include "Utils/Timer.h"

class BGPStateMachine{


private:
    
    BGPConnection* connection;

    BGPState* previousState;
    //Mandatory session attributes

    BGPState* currentState;

    


public:
    
    BGPStateMachine(BGPConnection* connection):connection(connection), currentState(new BGPStateIdle(this)){

    }

    virtual ~BGPStateMachine();



    bool HandleEvent(Event event);

    void ChangeState(BGPState* newState);


    BGPState* CurrentState(){return currentState;}
    BGPState* PreviousState(){return previousState;}

    //TODO print name of the current BGPState

};

#endif