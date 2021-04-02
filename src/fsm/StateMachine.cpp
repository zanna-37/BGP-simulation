#include "StateMachine.h"

template <class Connection, class State, class Event>
StateMachine<Connection, State, Event>::StateMachine(Connection* connection)
    : connection(connection) {
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

            Event event = eventQueue.front();
            eventQueue.pop();
            eventQueue_uniqueLock.unlock();

            if (event == Event::__SHUTDOWN) {
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

template <class Connection, class State, class Event>
StateMachine<Connection, State, Event>::~StateMachine() {
    enqueueEvent(TCPEvent::__SHUTDOWN);
    eventHandler->join();
    delete eventHandler;
    delete currentState;
    delete previousState;
}

template <class Connection, class State, class Event>
void StateMachine<Connection, State, Event>::enqueueEvent(Event event) {
    std::unique_lock<std::mutex> eventQueue_uniqueLock(eventQueue_mutex);
    eventQueue.push(event);
    eventQueue_ready.notify_one();
    eventQueue_uniqueLock.unlock();
}

template <class Connection, class State, class Event>
void StateMachine<Connection, State, Event>::changeState(State* newState) {
    assert(newState);

    delete previousState;
    previousState = currentState;
    L_VERBOSE("State change: " + currentState->NAME + " -> " + newState->NAME);

    currentState = newState;
}

template <class Connection, class State, class Event>
State* StateMachine<Connection, State, Event>::getCurrentState() const {
    return currentState;
}