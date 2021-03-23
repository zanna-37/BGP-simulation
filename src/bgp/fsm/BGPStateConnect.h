#ifndef BGPSTATECONNECT_H
#define BGPSTATECONNECT_H

#include "BGPStateMachine.h"
#include "BGPState.h"
#include "../BGPConnection.h"

class BGPStateConnect : public BGPState { 

private:
    

public: 
    //Constructor
    BGPStateConnect(BGPStateMachine* stateMachine) : BGPState(stateMachine){
        NAME = "CONNECT";
        L_DEBUG("State created: " + NAME);
    };

    //Deconstructor
    ~BGPStateConnect();

    void enter();
    void execute();
    void exit();

    bool onEvent(Event event);

};

#endif