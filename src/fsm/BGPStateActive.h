#ifndef BGPSTATEACTIVE_H
#define BGPSTATEACTIVE_H

#include "BGPStateMachine.h"
#include "BGPState.h"
#include "BGPConnection.h"

class BGPStateActive : public BGPState { 

private:
    

public: 
    //Constructor
    BGPStateActive(BGPStateMachine* stateMachine) : BGPState(stateMachine){};

    //Deconstructor
    ~BGPStateActive();

    void enter();
    void execute();
    void exit();

    bool OnEvent(Event event);

};

#endif