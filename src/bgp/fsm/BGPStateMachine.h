#ifndef BGPSTATEMACHINE_H
#define BGPSTATEMACHINE_H

#include <cassert>
#include <chrono>
#include <cstring>
#include <queue>
#include <thread>

#include "../../logger/Logger.h"
#include "../../utils/Timer.h"
#include "../BGPConnection.h"
#include "../Event.h"
#include "BGPState.h"

using namespace std::chrono_literals;

class BGPState;       // forward declaration
class BGPConnection;  // forward declaration
class Timer;          // forward declaration

class BGPStateMachine {
   private:
    BGPConnection* connection;
    BGPState*      previousState = nullptr;  // TODO check if we really need it

    std::thread*            eventHandler = nullptr;
    std::mutex              eventQueue_mutex;
    std::condition_variable eventQueue_ready;
    queue<Event>            eventQueue;

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

    // Mandatory session attributes
    Timer* connectRetryTimer = nullptr;
    Timer* holdTimer         = nullptr;
    Timer* keepAliveTimer    = nullptr;

    // Optional session attributes
    Timer* delayOpenTimer = nullptr;

    /**
     * Enqueue the event in the Finite state Machine event queue. It is called
     * from BGPConnection::enqueue event
     * @warning this method should be called by the BGPConnection. Use
     * BGPConnection::enqueueEvent instead
     * @param event the event triggered
     */
    void enqueueEvent(Event event);

    /**
     * Change the current state of the Finite State Machine
     * @param newState the newly created state that becomes the new current
     * state of the Finite State Machine
     */
    void changeState(BGPState* newState);

    /**
     * Increment the connectRetryCounter by one
     */
    void incrementConnectRetryCounter();

    /**
     * Stop the connectRetryTimer and resets it to the initial value
     */
    void resetConnectRetryTimer();
    /**
     * Stop the holdTimer and resets it to the initial value
     */
    void resetHoldTimer();
    /**
     * Stop the keepAliveTimer and resets it to the initial value
     */
    void resetKeepAliveTimer();
    /**
     * Stop the delayOpenTimer and resets it to the initial value
     */
    void resetDelayOpenTimer();

    /**
     *  Initialize the new timers to the default values
     */
    void initializeTimers();

    /**
     * Get the connectRetrycounter
     * @return the connectRetryCounter value
     */
    int getConnectRetryCounter() const { return connectRetryCounter; }
    /**
     * Set the connectRetryCounter
     * @param value the value of connectRetryCounter
     */
    void setConnectRetryCounter(int value) { connectRetryCounter = value; }

    /**
     * Get the dampPeerOscillations
     * @return the dampPeerOscillations value
     */
    bool getDampPeerOscillations() const { return dampPeerOscillations; }

    /**
     * Set the dampPeerOscillations
     * @param value the value of dampPeerOscillations
     */
    void setDampPeerOscillations(bool value) { dampPeerOscillations = value; }

    /**
     * Get the delayOpen
     * @return the delayOpen value
     */
    bool getDelayOpen() const { return delayOpen; }
    /**
     * Set the delayOpen
     * @param value the value of delayOpen
     */
    void setDelayOpen(bool value) { delayOpen = value; }

    /**
     * Get the sendNOTIFICATIONwithoutOPEN
     * @return the sendNOTIFICATIONwithoutOPEN value
     */
    bool getSendNOTIFICATIONwithoutOPEN() const {
        return sendNOTIFICATIONwithoutOPEN;
    }

    /**
     * Set the sendNOTIFICATIONwithoutOPEN
     * @param value the value of sendNOTIFICATIONwithoutOPEN
     */
    void setSendNOTIFICATIONwithoutOPEN(bool value) {
        sendNOTIFICATIONwithoutOPEN = value;
    }

    /**
     * Get the previousState of the Finite State Machine. Used for recreating
     * the flow of the FSM
     * @return the previousState value
     */
    BGPState* getPreviousState() const { return previousState; }
    /**
     * Set the previousState
     * @param value the value of previousState
     */
    void setPreviousState(BGPState* value) { previousState = value; }

    /**
     * Get the currentState of the Finite State Machine. Used for recreating the
     * flow of the FSM
     * @return the currentState value
     */
    BGPState* getCurrentState() const { return currentState; }

    /**
     * Set the currentState
     * @param value the value of currentState
     */
    void setCurrentState(BGPState* value) { currentState = value; }

    /**
     * Get the connectRetryTime default value. Used by connectRetryTimer
     * @return the connectRetryTime value
     */
    std::chrono::seconds getConnectRetryTime() const {
        return connectRetryTime;
    }

    /**
     * Set the connectRetryTime default value
     * @param value the value of connectRetryTime
     */
    void setConnectRetryTime(const std::chrono::seconds& value) {
        connectRetryTime = value;
    }

    /**
     * Get the holdTime default value. Used by holdTimer
     * @return the holdTime value
     */
    std::chrono::seconds getHoldTime() const { return holdTime; }

    /**
     * Set the holdTime default value
     * @param value the value of holdTime
     */
    void setHoldTime(const std::chrono::seconds& value) { holdTime = value; }

    /**
     * Get the keepAliveTime default value. Used by keepAliveTimer
     * @return the keepAliveTime value
     */
    std::chrono::seconds getKeepaliveTime() const { return keepaliveTime; }
    /**
     * Set the keepAliveTime default value
     * @param value the value of keepAliveTime
     */
    void setKeepaliveTime(const std::chrono::seconds& value) {
        keepaliveTime = value;
    }

    /**
     * Get the delayOpenTime default value. Used by delayOpenTimer
     * @return the delayOpenTime value
     */
    std::chrono::seconds getDelayOpenTime() const { return delayOpenTime; }
    /**
     * Set the delayOpenTime default value
     * @param value the value of delayOpenTime
     */
    void setDelayOpenTime(const std::chrono::seconds& value) {
        delayOpenTime = value;
    }

    // TODO print name of the current BGPState
};

#endif