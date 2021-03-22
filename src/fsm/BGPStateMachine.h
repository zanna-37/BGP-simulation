#ifndef BGPSTATEMACHINE_H
#define BGPSTATEMACHINE_H

#include <cassert>
#include <cstring>
#include <thread>


#include "BGPState.h"
#include "../BGPConnection.h"
#include "../Event.h"
#include "../Utils/Timer.h"

// debugging tokens

class BGPConnection; //forward declaration
class Timer;

class BGPStateMachine{


private:
    
    BGPConnection* connection;

    BGPState* previousState = nullptr;
    //Mandatory session attributes

    BGPState* currentState = nullptr;
    int connectRetryCounter = 0;
    // int connectRetryTime = 120;
    // int holdTime = 90;
    // int keepaliveTime = 30;
    //DEBUG
    int connectRetryTime = 3;
    int holdTime = 4;
    int keepaliveTime = 5;

    // https://gist.github.com/mcleary/b0bf4fa88830ff7c882d Timer implementation

    Timer* connectRetryTimer = nullptr;
    Timer* holdTimer = nullptr;
    Timer* keepAliveTimer = nullptr;
    
    //Optional attributes

    //   1) AcceptConnectionsUnconfiguredPeers
    //   2) AllowAutomaticStart
    //   3) AllowAutomaticStop
    //   4) CollisionDetectEstablishedState
    bool dampPeerOscillations = false;
    bool delayOpen = false;
    //   7) DelayOpenTime
    //   8) DelayOpenTimer
    //   9) IdleHoldTime
    //  10) IdleHoldTimer
    //  11) PassiveTcpEstablishment
    bool sendNOTIFICATIONwithoutOPEN = false;
    //  13) TrackTcpState

     

public:
    
    BGPStateMachine(BGPConnection* connection, BGPState* state);

    ~BGPStateMachine();



    bool handleEvent(Event event);

    void changeState(BGPState* newState);

    void incrementConnectRetryCounter();

    // BGPState* currentState(){return currentState;}
    // BGPState* previousState(){return previousState;}


    int getConnectRetryCounter() const { return connectRetryCounter; }
    void setConnectRetryCounter(int value) { connectRetryCounter = value; }

    bool getDampPeerOscillations() const { return dampPeerOscillations; }
    void setDampPeerOscillations(bool value) { dampPeerOscillations = value; }

    bool getDelayOpen() const { return delayOpen; }
    void setDelayOpen(bool value) { delayOpen = value; }

    int getHoldTime() const { return holdTime; }
    void setHoldTime(int value) { holdTime = value; }

    bool getSendNOTIFICATIONwithoutOPEN() const { return sendNOTIFICATIONwithoutOPEN; }
    void setSendNOTIFICATIONwithoutOPEN(bool value) { sendNOTIFICATIONwithoutOPEN = value; }

    int getConnectRetryTime() const { return connectRetryTime; }
    void setConnectRetryTime(int value) { connectRetryTime = value; }

    int getKeepaliveTime() const { return keepaliveTime; }
    void setKeepaliveTime(int value) { keepaliveTime = value; }

    BGPState* getPreviousState() const { return previousState; }
    void setPreviousState(BGPState* value) { previousState = value; }

    BGPState* getCurrentState() const { return currentState; }
    void setCurrentState(BGPState* value) { currentState = value; }

    //TODO print name of the current BGPState

};

#endif