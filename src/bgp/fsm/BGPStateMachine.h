#ifndef BGPSIMULATION_BGP_FSM_BGPSTATEMACHINE_H
#define BGPSIMULATION_BGP_FSM_BGPSTATEMACHINE_H

#include <chrono>
#include <string>

#include "../../fsm/StateMachine.h"
#include "../BGPEvent.h"

// forward declarations
#include "../BGPConnection.fwd.h"
#include "../BGPTimer.fwd.h"
#include "BGPState.fwd.h"

using namespace std::chrono_literals;


class BGPStateMachine : public StateMachine<BGPConnection, BGPState, BGPEvent> {
   public:
#ifdef DEBUG_GUARD
    constexpr static const std::chrono::seconds kConnectRetryTime_defaultVal =
        36s;
    constexpr static const std::chrono::seconds kHoldTime_defaultVal = 27s;
    constexpr static const std::chrono::seconds kHoldTime_large_defaultVal =
        72s;
    constexpr static const std::chrono::seconds kKeepaliveTime_defaultVal =
        kHoldTime_defaultVal / 3;
    constexpr static const std::chrono::seconds
        kMinASOriginationIntervalTime_defaultVal = 45s;
#else
    constexpr static const std::chrono::seconds kConnectRetryTime_defaultVal =
        120s;
    constexpr static const std::chrono::seconds kHoldTime_defaultVal = 90s;
    constexpr static const std::chrono::seconds kHoldTime_large_defaultVal =
        240s;
    constexpr static const std::chrono::seconds kKeepaliveTime_defaultVal =
        kHoldTime_defaultVal / 3;
    constexpr static const std::chrono::seconds
        kMinASOriginationIntervalTime_defaultVal = 150s;
#endif

    BGPStateMachine(BGPConnection* connection);

    ~BGPStateMachine();

    // Mandatory session attributes
    BGPTimer* connectRetryTimer             = nullptr;
    BGPTimer* holdTimer                     = nullptr;
    BGPTimer* keepAliveTimer                = nullptr;
    BGPTimer* minASOriginationIntervalTimer = nullptr;

    // Optional session attributes
    BGPTimer* delayOpenTimer = nullptr;

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
     * Stop the minASOriginationIntervalTimer and resets it to the initial value
     */
    void resetMinASOriginationIntervalTimer();
    /**
     * Stop the delayOpenTimer and resets it to the initial value
     */
    void resetDelayOpenTimer();

    /**
     *  Initialize the new timers to the default values
     */
    void initializeTimers();

    /**
     * @brief Initialize the negotiated time values used for this connection
     *
     */
    void initializeTimes();

    /**
     * Get the connectRetrycounter
     * @return the connectRetryCounter value
     */
    int  getConnectRetryCounter() const { return connectRetryCounter; }
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
     * @brief Get the holdTime negotiated value.
     * @return the holdTime value
     */
    std::chrono::seconds getNegotiatedHoldTime() const {
        return negotiatedHoldTime;
    }

    /**
     * Set the holdTime negotiated value
     * @param value the value of negotiatedHoldTime
     */
    void setNegotiatedHoldTime(const std::chrono::seconds& value) {
        negotiatedHoldTime = value;
    }

    /**
     * Get the keepAliveTime default value. Used by keepAliveTimer
     * @return the keepAliveTime value
     */
    std::chrono::seconds getNegotiatedKeepaliveTime() const {
        return negotiatedKeepaliveTime;
    }
    /**
     * Set the keepAliveTime default value
     * @param value the value of keepAliveTime
     */
    void setNegotiatedKeepaliveTime(const std::chrono::seconds& value) {
        negotiatedKeepaliveTime = value;
    }

    /**
     * Get the minASOriginationIntervalTime default value. Used by
     * minASOriginationIntervalTimer
     * @return the minASOriginationIntervalTime value
     */
    std::chrono::seconds getMinASOriginationIntervalTime() const {
        return minASOriginationIntervalTime;
    }
    /**
     * Set the minASOriginationIntervalTime default value
     * @param value the value of minASOriginationIntervalTime
     */
    void setMinASOriginationIntervalTime(const std::chrono::seconds& value) {
        minASOriginationIntervalTime = value;
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
    void                 setDelayOpenTime(const std::chrono::seconds& value) {
        delayOpenTime = value;
    }

    /**
     * @brief Get the Accept Connections Unconfigured Peers object
     *
     * @return false
     */
    bool getAcceptConnectionsUnconfiguredPeers() const {
        return AcceptConnectionsUnconfiguredPeers;
    }
    /**
     * @brief Get the Allow Automatic Start object
     *
     * @return false
     */
    bool getAllowAutomaticStart() const { return AllowAutomaticStart; }
    /**
     * @brief Get the Allow Automatic Stop object
     *
     * @return false
     */
    bool getAllowAutomaticStop() const { return AllowAutomaticStop; }
    /**
     * @brief Get the Collision Detect Established State object
     *
     * @return false
     */
    bool getCollisionDetectEstablishedState() const {
        return CollisionDetectEstablishedState;
    }
    /**
     * @brief Get the Idle Hold Time object
     *
     * @return false
     */
    bool getIdleHoldTime() const { return IdleHoldTime; }
    /**
     * @brief Get the Idle Hold Timer object
     *
     * @return false
     */
    bool getIdleHoldTimer() const { return IdleHoldTimer; }
    /**
     * @brief Get the Passive Tcp Establishment object
     *
     * @return false
     */
    bool getPassiveTcpEstablishment() const { return PassiveTcpEstablishment; }
    /**
     * @brief Get the Track Tcp State object
     *
     * @return false
     */
    bool getTrackTcpState() const { return TrackTcpState; }


   protected:
    std::string toString() override;

    // TODO print name of the current BGPState

   private:
    // TODO from the RFC, the following "ConnectRetryTime" "HoldTime"
    // "KeepaliveTime" should be the default values, but they are already set
    // above, Do we need to follow word by word the RFC?
    std::chrono::seconds connectRetryTime                       = 0s;
    std::chrono::seconds negotiatedHoldTime                     = 0s;
    std::chrono::seconds negotiatedKeepaliveTime                = 0s;
    std::chrono::seconds minASOriginationIntervalTime           = 0s;

    // Mandatory session attributes
    int connectRetryCounter = 0;

    // Optional attributes

    //  1) AcceptConnectionsUnconfiguredPeers
    //  2) AllowAutomaticStart
    //  3) AllowAutomaticStop
    //  4) CollisionDetectEstablishedState
    //  5) DampPeerOscillations
    //  6) DelayOpen
    //  7) DelayOpenTime
    //  8) DelayOpenTimer
    //  9) IdleHoldTime
    // 10) IdleHoldTimer
    // 11) PassiveTcpEstablishment
    // 12) SendNOTIFICATIONwithoutOPEN
    // 13) TrackTcpState
    bool                 AcceptConnectionsUnconfiguredPeers = false;
    bool                 AllowAutomaticStart                = false;
    bool                 AllowAutomaticStop                 = false;
    bool                 CollisionDetectEstablishedState    = false;
    bool                 dampPeerOscillations               = false;
    bool                 delayOpen                          = false;
    std::chrono::seconds delayOpenTime                      = 0s;  // TODO
    bool                 IdleHoldTime                       = false;
    bool                 IdleHoldTimer                      = false;
    bool                 PassiveTcpEstablishment            = false;
    bool                 sendNOTIFICATIONwithoutOPEN        = false;
    bool                 TrackTcpState                      = false;
};

#endif  // BGPSIMULATION_BGP_FSM_BGPSTATEMACHINE_H
