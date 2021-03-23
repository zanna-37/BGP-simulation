#include "BGPStateMachine.h"

BGPStateMachine :: BGPStateMachine(BGPConnection* connection):connection(connection){

    initializeTimers();
    currentState = new BGPStateIdle(this);

    L_DEBUG("State Machine Created. Initial state: " + currentState->NAME);
}
BGPStateMachine :: ~BGPStateMachine(){

    delete currentState;
    delete previousState;

    //delete timers
    delete connectRetryTimer;
    delete keepAliveTimer;
    delete holdTimer;
    delete delayOpenTimer;

}

bool BGPStateMachine :: handleEvent(Event event){

    L_DEBUG("Event Occurred: " + to_string(event));

    if (currentState && currentState->onEvent(event)){
        return true;
    }
    
    return false;
}

void BGPStateMachine :: changeState(BGPState* newState){

    assert (newState);
    L_INFO("State change: " + currentState->NAME + " -> " + newState->NAME);

    if(previousState != nullptr){
        delete previousState;
    }
    
    previousState = currentState;
    currentState->exit();
    currentState = newState;

    currentState->enter();

}

void BGPStateMachine :: incrementConnectRetryCounter(){

    connectRetryCounter +=1;
    L_DEBUG("connectRetryCounter incremented. Current value: " + to_string(connectRetryCounter));
}

void BGPStateMachine :: resetConnectRetryTimer(){
    if(connectRetryTimer != nullptr){
        connectRetryTimer->stop();
        delete connectRetryTimer;
    }
    
    connectRetryTimer = new Timer("ConnectRetryTimer", ConnectRetryTimer_Expires, this, connectRetryTime);
}

void BGPStateMachine :: resetHoldTimer(){
    if(holdTimer != nullptr){
        holdTimer->stop();
        delete holdTimer;
    }
    
    holdTimer = new Timer("HoldTimer", HoldTimer_Expires, this, holdTime);
}

void BGPStateMachine :: resetKeepAliveTimer(){
    if(keepAliveTimer != nullptr){
        keepAliveTimer->stop();
        delete keepAliveTimer;
    }
    
    keepAliveTimer = new Timer("KeepAliveTimer", KeepaliveTimer_Expires, this, keepaliveTime);
}

void BGPStateMachine :: resetDelayOpenTimer(){
    if(delayOpenTimer != nullptr){
        delayOpenTimer->stop();
        delete delayOpenTimer;
    }    
    delayOpenTimer = new Timer("DelayOpenTimer", DelayOpenTimer_Expires, this, delayOpenTime);
}

void BGPStateMachine :: initializeTimers(){
    resetConnectRetryTimer();
    resetHoldTimer();
    resetKeepAliveTimer();
    resetDelayOpenTimer();
}

