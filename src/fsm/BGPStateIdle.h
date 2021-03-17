#ifndef BGPSTATEIDLE_H
#define BGPSTATEIDLE_H

#include "BGPStateMachine.h"
#include "BGPState.h"
#include "../BGPConnection.h"

class BGPStateIdle : public BGPState { 

private:
    

public: 
    //Constructor
    BGPStateIdle(BGPStateMachine* stateMachine): BGPState(stateMachine){};

    //Deconstructor
    ~BGPStateIdle();

    void enter();
    void execute();
    void exit();

    bool OnEvent(Event);

};

#endif