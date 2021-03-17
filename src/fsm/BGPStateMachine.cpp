#include "BGPStateMachine.h"

BGPStateMachine :: ~BGPStateMachine(){

    delete currentState;
    delete previousState;

}

bool BGPStateMachine :: HandleEvent(Event event){

    if (currentState && currentState->OnEvent(event)){
        return true;
    }
    
    return false;
}

void BGPStateMachine :: ChangeState(BGPState* newState){

    assert (newState);

    delete previousState;
    previousState = currentState;
    currentState->exit();
    currentState = newState;

    currentState->enter();

}

void BGPStateMachine :: incrementConnectRetryCounter(){

    ConnectRetryCounter +=1;
}