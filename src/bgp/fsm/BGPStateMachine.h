#ifndef BGPSTATEMACHINE_H
#define BGPSTATEMACHINE_H

#include <cassert>
#include <chrono>
#include <cstring>
#include <thread>

#include "../../logger/Logger.h"
#include "../../utils/Timer.h"
#include "../BGPConnection.h"
#include "../Event.h"
#include "BGPState.h"

// debugging tokens

class BGPConnection;  // forward declaration
class Timer;

using namespace std::chrono_literals;

class BGPStateMachine {
   private:
    BGPConnection* connection;

    BGPState* previousState = nullptr;
    // Mandatory session attributes

    BGPState* currentState        = nullptr;
    int       connectRetryCounter = 0;
#ifdef DEBUG_GUARD
    std::chrono::seconds connectRetryTime = 3s;
    std::chrono::seconds holdTime         = 4s;
    std::chrono::seconds keepaliveTime    = 5s;
#else
    std::chrono::seconds connectRetryTime = 120s;
    std::chrono::seconds holdTime         = 90s;
    std::chrono::seconds keepaliveTime    = 30s;
#endif

    // https://gist.github.com/mcleary/b0bf4fa88830ff7c882d Timer implementation


    // Optional attributes

    //   1) AcceptConnectionsUnconfiguredPeers
    //   2) AllowAutomaticStart
    //   3) AllowAutomaticStop
    //   4) CollisionDetectEstablishedState
    bool                 dampPeerOscillations = false;
    bool                 delayOpen            = false;
    std::chrono::seconds delayOpenTime        = 0s;  // TODO
    //   9) IdleHoldTime
    //  10) IdleHoldTimer
    //  11) PassiveTcpEstablishment
    bool sendNOTIFICATIONwithoutOPEN = false;
    //  13) TrackTcpState


   public:
    BGPStateMachine(BGPConnection* connection);

    ~BGPStateMachine();

    // mandatory
    Timer* connectRetryTimer = nullptr;
    Timer* holdTimer         = nullptr;
    Timer* keepAliveTimer    = nullptr;

    // optional
    Timer* delayOpenTimer = nullptr;

    bool handleEvent(Event event);

    void changeState(BGPState* newState);

    void incrementConnectRetryCounter();

    // BGPState* currentState(){return currentState;}
    // BGPState* previousState(){return previousState;}

    void resetConnectRetryTimer();
    void resetHoldTimer();
    void resetKeepAliveTimer();
    void resetDelayOpenTimer();

    void initializeTimers();


    int  getConnectRetryCounter() const { return connectRetryCounter; }
    void setConnectRetryCounter(int value) { connectRetryCounter = value; }

    bool getDampPeerOscillations() const { return dampPeerOscillations; }
    void setDampPeerOscillations(bool value) { dampPeerOscillations = value; }

    bool getDelayOpen() const { return delayOpen; }
    void setDelayOpen(bool value) { delayOpen = value; }

    bool getSendNOTIFICATIONwithoutOPEN() const {
        return sendNOTIFICATIONwithoutOPEN;
    }
    void setSendNOTIFICATIONwithoutOPEN(bool value) {
        sendNOTIFICATIONwithoutOPEN = value;
    }

    BGPState* getPreviousState() const { return previousState; }
    void      setPreviousState(BGPState* value) { previousState = value; }

    BGPState* getCurrentState() const { return currentState; }
    void      setCurrentState(BGPState* value) { currentState = value; }

    std::chrono::seconds getConnectRetryTime() const {
        return connectRetryTime;
    }
    void setConnectRetryTime(const std::chrono::seconds& value) {
        connectRetryTime = value;
    }

    std::chrono::seconds getHoldTime() const { return holdTime; }
    void setHoldTime(const std::chrono::seconds& value) { holdTime = value; }

    std::chrono::seconds getKeepaliveTime() const { return keepaliveTime; }
    void                 setKeepaliveTime(const std::chrono::seconds& value) {
        keepaliveTime = value;
    }

    std::chrono::seconds getDelayOpenTime() const { return delayOpenTime; }
    void                 setDelayOpenTime(const std::chrono::seconds& value) {
        delayOpenTime = value;
    }

    // TODO print name of the current BGPState
};

#endif