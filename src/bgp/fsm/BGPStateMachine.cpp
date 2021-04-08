#include "BGPStateMachine.h"

BGPStateMachine::BGPStateMachine(BGPConnection* connection)
    : StateMachine(connection) {
    initializeTimers();
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
                to_string(connectRetryCounter));
}

void BGPStateMachine::resetConnectRetryTimer() {
    if (connectRetryTimer != nullptr) {
        connectRetryTimer->stop();
        delete connectRetryTimer;
    }

    connectRetryTimer = new BGPTimer("ConnectRetryTimer",
                                     this,
                                     BGPEvent::ConnectRetryTimer_Expires,
                                     connectRetryTime);
}

void BGPStateMachine::resetHoldTimer() {
    if (holdTimer != nullptr) {
        holdTimer->stop();
        delete holdTimer;
    }

    holdTimer =
        new BGPTimer("HoldTimer", this, BGPEvent::HoldTimer_Expires, holdTime);
}

void BGPStateMachine::resetKeepAliveTimer() {
    if (keepAliveTimer != nullptr) {
        keepAliveTimer->stop();
        delete keepAliveTimer;
    }

    keepAliveTimer = new BGPTimer("KeepAliveTimer",
                                  this,
                                  BGPEvent::KeepaliveTimer_Expires,
                                  keepaliveTime);
}

void BGPStateMachine::resetDelayOpenTimer() {
    if (delayOpenTimer != nullptr) {
        delayOpenTimer->stop();
        delete delayOpenTimer;
    }
    delayOpenTimer = new BGPTimer("DelayOpenTimer",
                                  this,
                                  BGPEvent::DelayOpenTimer_Expires,
                                  delayOpenTime);
}

void BGPStateMachine::initializeTimers() {
    resetConnectRetryTimer();
    resetHoldTimer();
    resetKeepAliveTimer();
    resetDelayOpenTimer();
}