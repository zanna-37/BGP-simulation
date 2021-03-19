#include "BGPStateMachine.h"

BGPStateMachine :: BGPStateMachine(BGPConnection* connection, BGPState* state):connection(connection), currentState(state){

    connectRetryTimer = new Timer("ConnectRetryTimer");
    holdTimer = new Timer("HoldTimer");
    keepAliveTimer = new Timer("keepAliveTimer");

    //timers start

    connectRetryTimer->start(std::chrono::seconds(connectRetryTime), this, ConnectRetryTimer_Expires);
    connectRetryTimer->join();

}
BGPStateMachine :: ~BGPStateMachine(){

    delete currentState;
    delete previousState;

    //delete timers
    delete connectRetryTimer;
    delete keepAliveTimer;
    delete holdTimer;

}

bool BGPStateMachine :: handleEvent(Event event){

    std::cout << "handleEvent" << std::endl;

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