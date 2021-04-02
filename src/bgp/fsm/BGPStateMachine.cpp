#include "BGPStateMachine.h"

template <class Connection, class State, class Event>
BGPStateMachine<Connection, State, Event>::BGPStateMachine(
    Connection* connection)
    : StateMachine<Connection, State, Event>(connection) {
    initializeTimers();
    this->currentState = new BGPStateIdle(this);
    L_DEBUG("State Machine Created. Initial state: " +
            this->currentState->NAME);
}
template <class Connection, class State, class Event>
BGPStateMachine<Connection, State, Event>::~BGPStateMachine() {
    // ~StateMachine<Connection, State, Event>();
    // delete timers
    delete connectRetryTimer;
    delete keepAliveTimer;
    delete holdTimer;
    delete delayOpenTimer;
}
template <class Connection, class State, class Event>
void BGPStateMachine<Connection, State, Event>::incrementConnectRetryCounter() {
    connectRetryCounter += 1;
    L_DEBUG("connectRetryCounter incremented. Current value: " +
            to_string(connectRetryCounter));
}

template <class Connection, class State, class Event>
void BGPStateMachine<Connection, State, Event>::resetConnectRetryTimer() {
    if (connectRetryTimer != nullptr) {
        connectRetryTimer->stop();
        delete connectRetryTimer;
    }

    connectRetryTimer = new Timer("ConnectRetryTimer",
                                  this,
                                  BGPEvent::ConnectRetryTimer_Expires,
                                  connectRetryTime);
}

template <class Connection, class State, class Event>
void BGPStateMachine<Connection, State, Event>::resetHoldTimer() {
    if (holdTimer != nullptr) {
        holdTimer->stop();
        delete holdTimer;
    }

    holdTimer =
        new Timer("HoldTimer", this, BGPEvent::HoldTimer_Expires, holdTime);
}

template <class Connection, class State, class Event>
void BGPStateMachine<Connection, State, Event>::resetKeepAliveTimer() {
    if (keepAliveTimer != nullptr) {
        keepAliveTimer->stop();
        delete keepAliveTimer;
    }

    keepAliveTimer = new Timer("KeepAliveTimer",
                               this,
                               BGPEvent::KeepaliveTimer_Expires,
                               keepaliveTime);
}

template <class Connection, class State, class Event>
void BGPStateMachine<Connection, State, Event>::resetDelayOpenTimer() {
    if (delayOpenTimer != nullptr) {
        delayOpenTimer->stop();
        delete delayOpenTimer;
    }
    delayOpenTimer = new Timer("DelayOpenTimer",
                               this,
                               BGPEvent::DelayOpenTimer_Expires,
                               delayOpenTime);
}

template <class Connection, class State, class Event>
void BGPStateMachine<Connection, State, Event>::initializeTimers() {
    resetConnectRetryTimer();
    resetHoldTimer();
    resetKeepAliveTimer();
    resetDelayOpenTimer();
}
template class BGPStateMachine<BGPConnection, BGPState, BGPEvent>;