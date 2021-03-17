#ifndef BGPSTATEOPENCONFIRM_H
#define BGPSTATEOPENCONFIRM_H


#include "BGPStateMachine.h"
#include "BGPState.h"
#include "BGPConnection.h"

class BGPStateOpenConfirm : public BGPState { 

private:
    

public: 
    //Constructor
    BGPStateOpenConfirm(BGPStateMachine* stateMachine) : BGPState(stateMachine){};

    //Deconstructor
    ~BGPStateOpenConfirm();

    void enter();
    void execute();
    void exit();

    bool OnEvent(Event event);

};

#endif