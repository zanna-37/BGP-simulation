#include "BGPStateMachine.h"

BGPStateMachine :: ~BGPStateMachine(){

    delete currentState;
    delete previousState;

    //delete timers
    delete connectRetryTimer;
    delete keepAliveTimer;
    delete holdTimer;

}

bool BGPStateMachine :: handleEvent(Event event){

    if (currentState && currentState->onEvent(event)){
        return true;
    }
    
    return false;
}

void BGPStateMachine :: changeState(BGPState* newState){

    assert (newState);

    delete previousState;
    previousState = currentState;
    currentState->exit();
    currentState = newState;

    currentState->enter();

}

void BGPStateMachine :: incrementConnectRetryCounter(){

    connectRetryCounter +=1;
}