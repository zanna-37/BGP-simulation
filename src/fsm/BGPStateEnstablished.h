#ifndef BGPSTATEENSTABLISHED_H
#define BGPSTATEENSTABLISHED_H

#include "BGPStateMachine.h"
#include "BGPState.h"
#include "BGPConnection.h"

class BGPStateEnstablished : public BGPState { 

private:
    

public: 
    //Constructor
    BGPStateEnstablished(BGPStateMachine* stateMachine) : BGPState(stateMachine){};

    //Deconstructor
    ~BGPStateEnstablished();

    void enter();
    void execute();
    void exit();

    bool OnEvent(Event event);

};

#endif