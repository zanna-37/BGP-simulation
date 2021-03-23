#ifndef BGPSTATEENSTABLISHED_H
#define BGPSTATEENSTABLISHED_H

#include "BGPStateMachine.h"
#include "BGPState.h"
#include "../BGPConnection.h"

class BGPStateEnstablished : public BGPState { 

private:
    

public: 
    //Constructor
    BGPStateEnstablished(BGPStateMachine* stateMachine) : BGPState(stateMachine){
        NAME = "ENSTABLISHED";
        L_DEBUG("State created: " + NAME);
    };

    //Deconstructor
    ~BGPStateEnstablished();

    void enter();
    void execute();
    void exit();

    bool onEvent(Event event);

};

#endif