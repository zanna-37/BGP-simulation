#ifndef BGPSTATEMACHINE_H
#define BGPSTATEMACHINE_H

#include <cassert>


#include "fsm/BGPState.h"
#include "BGPConnection.h"
#include "Event.h"
#include "Utils/Timer.h"

class BGPStateMachine{


private:
    
    BGPConnection* connection;

    BGPState* previousState;
    //Mandatory session attributes

    BGPState* currentState;
    int ConnectRetryCounter = 0;
    int ConnectRetryTime = 120;
    int HoldTime = 90;
    int KeepaliveTime = 30;

    // https://gist.github.com/mcleary/b0bf4fa88830ff7c882d Timer implementation
    Timer ConnectRetryTimer;
    Timer HoldTimer;
    Timer KeepaliveTimer;
    
    //Optional attributes

    //   1) AcceptConnectionsUnconfiguredPeers
    //   2) AllowAutomaticStart
    //   3) AllowAutomaticStop
    //   4) CollisionDetectEstablishedState
    bool DampPeerOscillations;
    bool DelayOpen;
    //   7) DelayOpenTime
    //   8) DelayOpenTimer
    //   9) IdleHoldTime
    //  10) IdleHoldTimer
    //  11) PassiveTcpEstablishment
    bool SendNOTIFICATIONwithoutOPEN;
    //  13) TrackTcpState

     

    


public:
    
    BGPStateMachine(BGPConnection* connection):connection(connection), currentState(new BGPStateIdle(this)){

    }

    virtual ~BGPStateMachine();



    bool HandleEvent(Event event);

    void ChangeState(BGPState* newState);

    void incrementConnectRetryCounter();

    BGPState* CurrentState(){return currentState;}
    BGPState* PreviousState(){return previousState;}

    int connectRetryCounter() const { return ConnectRetryCounter; }
    void setConnectRetryCounter(int value) { ConnectRetryCounter = value; }

    bool dampPeerOscillations() const { return DampPeerOscillations; }
    void setDampPeerOscillations(bool value) { DampPeerOscillations = value; }

    bool delayOpen() const { return DelayOpen; }
    void setDelayOpen(bool value) { DelayOpen = value; }

    int holdTime() const { return HoldTime; }
    void setHoldTime(int value) { HoldTime = value; }

    bool sendNOTIFICATIONwithoutOPEN() const { return SendNOTIFICATIONwithoutOPEN; }
    void setSendNOTIFICATIONwithoutOPEN(bool value) { SendNOTIFICATIONwithoutOPEN = value; }

    //TODO print name of the current BGPState

};

#endif