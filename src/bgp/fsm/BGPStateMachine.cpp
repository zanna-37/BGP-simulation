#include "BGPStateMachine.h"

#include "../../entities/Router.h"
#include "../../fsm/StateMachine.h"
#include "../../logger/Logger.h"
#include "../BGPConnection.h"
#include "../BGPEvent.h"
#include "../BGPTimer.h"
#include "BGPState.h"
#include "BGPStateIdle.h"
#include "IpAddress.h"


BGPStateMachine::BGPStateMachine(BGPConnection* connection)
    : StateMachine(connection, "BGPfsm") {
    initializeTimers();
    changeState(new BGPStateIdle(this));
}

BGPStateMachine::~BGPStateMachine() {
    // ~StateMachine();
    // delete timers
    delete connectRetryTimer;
    delete keepAliveTimer;
    delete holdTimer;
    delete delayOpenTimer;
}
void BGPStateMachine::incrementConnectRetryCounter() {
    connectRetryCounter += 1;
    L_DEBUG(connection->owner->ID,
            "connectRetryCounter incremented. Current value: " +
                std::to_string(connectRetryCounter));
}

void BGPStateMachine::resetConnectRetryTimer() {
    if (connectRetryTimer != nullptr) {
        connectRetryTimer->stop();
        delete connectRetryTimer;
    }
    BGPEvent event = {
        BGPEventList::ConnectRetryTimer_Expires,
        nullptr,
    };
    connectRetryTimer = new BGPTimer(
        "ConnectRetryTimer", this, event, kConnectRetryTime_defaultVal);
}

void BGPStateMachine::resetHoldTimer() {
    if (holdTimer != nullptr) {
        holdTimer->stop();
        delete holdTimer;
    }
    BGPEvent event = {
        BGPEventList::HoldTimer_Expires,
        nullptr,
    };
    holdTimer = new BGPTimer("HoldTimer", this, event, kHoldTime_defaultVal);
}

void BGPStateMachine::resetKeepAliveTimer() {
    if (keepAliveTimer != nullptr) {
        keepAliveTimer->stop();
        delete keepAliveTimer;
    }
    BGPEvent event = {
        BGPEventList::KeepaliveTimer_Expires,
        nullptr,
    };
    keepAliveTimer =
        new BGPTimer("KeepAliveTimer", this, event, kKeepaliveTime_defaultVal);
}

void BGPStateMachine::resetDelayOpenTimer() {
    if (delayOpenTimer != nullptr) {
        delayOpenTimer->stop();
        delete delayOpenTimer;
    }
    BGPEvent event = {
        BGPEventList::DelayOpenTimer_Expires,
        nullptr,
    };

    delayOpenTimer = new BGPTimer("DelayOpenTimer", this, event, delayOpenTime);
}

void BGPStateMachine::initializeTimers() {
    resetConnectRetryTimer();
    resetHoldTimer();
    resetKeepAliveTimer();
    resetDelayOpenTimer();
}

std::string BGPStateMachine::toString() {
    if (connection) {
        return connection->srcAddr.toString() + " " +
               connection->dstAddr.toString();
    } else {
        return "";
    }
}
