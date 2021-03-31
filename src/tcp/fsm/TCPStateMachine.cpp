#include "TCPStateMachine.h"

TCPStateMachine::TCPStateMachine(TCPConnection* connection)
    : connection(connection) {
    currentState = new TCPStateClosed(this);


    eventHandler = new std::thread([&]() {
        bool running = true;

        while (running) {
            std::unique_lock<std::mutex> eventQueue_uniqueLock(
                eventQueue_mutex);

            while (eventQueue.empty()) {
                eventQueue_ready.wait(eventQueue_uniqueLock);
                if (eventQueue.empty()) {
                    L_DEBUG("Spurious wakeup");
                }
            }

            TCPEvent event = eventQueue.front();
            eventQueue.pop();
            eventQueue_uniqueLock.unlock();

            if (event == __INTERNAL_SHUTDOWN) {
                running = false;
            } else {
                L_DEBUG("Passing event " + getEventName(event) +
                        " to the current state (" + currentState->NAME + ")");
                TCPState* hanglingState_forlogs =
                    currentState;  // only used in the logs
                bool result = currentState && currentState->onEvent(event);
                L_DEBUG("Event " + getEventName(event) +
                        (result ? " handled" : " NOT handled") + " by " +
                        hanglingState_forlogs->NAME);
            }
        }
        L_DEBUG("Turning off eventHandler");
    });
}


TCPStateMachine::~TCPStateMachine() {
    enqueueEvent(__INTERNAL_SHUTDOWN);
    eventHandler->join();
    delete eventHandler;

    delete currentState;
}


void TCPStateMachine::enqueueEvent(TCPEvent event) {
    std::unique_lock<std::mutex> eventQueue_uniqueLock(eventQueue_mutex);
    eventQueue.push(event);
    eventQueue_ready.notify_one();
    eventQueue_uniqueLock.unlock();
}

void TCPStateMachine::changeState(TCPState* newState) {
    assert(newState);

    delete currentState;

    currentState = newState;
}