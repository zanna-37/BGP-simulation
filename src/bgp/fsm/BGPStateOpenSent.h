#ifndef BGPSTATEOPENSENT_H
#define BGPSTATEOPENSENT_H

#include "BGPStateMachine.h"
#include "BGPState.h"
#include "../BGPConnection.h"

class BGPStateOpenSent : public BGPState { 

private:
    

public: 
    //Constructor
    BGPStateOpenSent(BGPStateMachine* stateMachine) : BGPState(stateMachine){};

    //Deconstructor
    ~BGPStateOpenSent();

    void enter();
    void execute();
    void exit();

    bool onEvent(Event event);

};

#endif 