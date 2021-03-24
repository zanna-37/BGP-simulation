#include "BGPStateMachine.h"

BGPStateMachine ::BGPStateMachine(BGPConnection* connection)
    : connection(connection) {
    initializeTimers();
    currentState = new BGPStateIdle(this);


    eventHandler = new std::thread([&]() {
        bool running = true;
        while (running) {
            unique_lock<std::mutex> eventQueue_uniqueLock(eventQueue_mutex);

            while (eventQueue.empty()) {
                eventQueue_ready.wait(eventQueue_uniqueLock);
                if (eventQueue.empty()) {
                    L_DEBUG("Spurious wakeup");
                }
            }

            Event event = eventQueue.front();
            eventQueue.pop();
            eventQueue_uniqueLock.unlock();

            if (event == __INTERNAL_SHUTDOWN) {
                running = false;
            } else {
                L_DEBUG("Passing event " + getEventName(event) +
                        " to the current state (" + currentState->NAME + ")");
                BGPState* hanglingState_forlogs =
                    currentState;  // only used in the logs
                bool result = currentState && currentState->onEvent(event);
                L_DEBUG("Event " + getEventName(event) +
                        (result ? " handled" : " NOT handled") + " by " +
                        hanglingState_forlogs->NAME);
            }
        }
        L_DEBUG("Turning off eventHandler");
    });

    L_DEBUG("State Machine Created. Initial state: " + currentState->NAME);
}
BGPStateMachine ::~BGPStateMachine() {
    enqueueEvent(__INTERNAL_SHUTDOWN);
    eventHandler->join();
    delete eventHandler;

    delete currentState;
    delete previousState;

    // delete timers
    delete connectRetryTimer;
    delete keepAliveTimer;
    delete holdTimer;
    delete delayOpenTimer;
}

void BGPStateMachine ::enqueueEvent(Event event) {
    unique_lock<std::mutex> eventQueue_uniqueLock(eventQueue_mutex);
    L_DEBUG("Enqueueing event " + getEventName(event));
    eventQueue.push(event);
    eventQueue_ready.notify_one();
    eventQueue_uniqueLock.unlock();
}

void BGPStateMachine ::changeState(BGPState* newState) {
    assert(this_thread::get_id() == eventHandler->get_id());

    assert(newState);
    L_VERBOSE("State change: " + currentState->NAME + " -> " + newState->NAME);

    delete previousState;

    previousState = currentState;
    currentState = newState;
}

void BGPStateMachine ::incrementConnectRetryCounter() {
    connectRetryCounter += 1;
    L_DEBUG("connectRetryCounter incremented. Current value: " +
            to_string(connectRetryCounter));
}

void BGPStateMachine ::resetConnectRetryTimer() {
    if (connectRetryTimer != nullptr) {
        connectRetryTimer->stop();
        delete connectRetryTimer;
    }

    connectRetryTimer = new Timer(
        "ConnectRetryTimer", this, ConnectRetryTimer_Expires, connectRetryTime);
}

void BGPStateMachine ::resetHoldTimer() {
    if (holdTimer != nullptr) {
        holdTimer->stop();
        delete holdTimer;
    }

    holdTimer = new Timer("HoldTimer", this, HoldTimer_Expires, holdTime);
}

void BGPStateMachine ::resetKeepAliveTimer() {
    if (keepAliveTimer != nullptr) {
        keepAliveTimer->stop();
        delete keepAliveTimer;
    }

    keepAliveTimer = new Timer(
        "KeepAliveTimer", this, KeepaliveTimer_Expires, keepaliveTime);
}

void BGPStateMachine ::resetDelayOpenTimer() {
    if (delayOpenTimer != nullptr) {
        delayOpenTimer->stop();
        delete delayOpenTimer;
    }
    delayOpenTimer = new Timer(
        "DelayOpenTimer", this, DelayOpenTimer_Expires, delayOpenTime);
}

void BGPStateMachine ::initializeTimers() {
    resetConnectRetryTimer();
    resetHoldTimer();
    resetKeepAliveTimer();
    resetDelayOpenTimer();
}
