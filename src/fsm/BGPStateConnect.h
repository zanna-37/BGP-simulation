#ifndef BGPSTATECONNECT_H
#define BGPSTATECONNECT_H

#include "BGPStateMachine.h"
#include "BGPState.h"
#include "../BGPConnection.h"

class BGPStateConnect : public BGPState { 

private:
    

public: 
    //Constructor
    BGPStateConnect(BGPStateMachine* stateMachine) : BGPState(stateMachine){};

    //Deconstructor
    ~BGPStateConnect();

    void enter();
    void execute();
    void exit();

    bool OnEvent(Event event);

};

#endif