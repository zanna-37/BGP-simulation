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
    L_DEBUG("connectRetryCounter incremented. Current value: " +
            to_string(connectRetryCounter));
}

void BGPStateMachine::resetConnectRetryTimer() {
    if (connectRetryTimer != nullptr) {
        connectRetryTimer->stop();
        delete connectRetryTimer;
    }

    connectRetryTimer = new Timer("ConnectRetryTimer",
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
        new Timer("HoldTimer", this, BGPEvent::HoldTimer_Expires, holdTime);
}

void BGPStateMachine::resetKeepAliveTimer() {
    if (keepAliveTimer != nullptr) {
        keepAliveTimer->stop();
        delete keepAliveTimer;
    }

    keepAliveTimer = new Timer("KeepAliveTimer",
                               this,
                               BGPEvent::KeepaliveTimer_Expires,
                               keepaliveTime);
}

void BGPStateMachine::resetDelayOpenTimer() {
    if (delayOpenTimer != nullptr) {
        delayOpenTimer->stop();
        delete delayOpenTimer;
    }
    delayOpenTimer = new Timer("DelayOpenTimer",
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