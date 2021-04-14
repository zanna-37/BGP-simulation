#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <cassert>
#include <condition_variable>
#include <queue>
#include <thread>

#include "../logger/Logger.h"
template <typename Connection, typename State, typename Event>
class StateMachine {
   private:
    std::thread*            eventHandler = nullptr;
    std::mutex              eventQueue_mutex;
    std::condition_variable eventQueue_ready;
    queue<Event>            eventQueue;
    bool                    running = false;

   public:
    Connection* connection    = nullptr;
    State*      previousState = nullptr;
    State*      currentState  = nullptr;

    StateMachine(Connection* connection) : connection(connection) {}

    ~StateMachine() {
        running = false;

        unique_lock<std::mutex> stateMachineEventQueue_uniqueLock(
            eventQueue_mutex);
        eventQueue_ready.notify_one();
        stateMachineEventQueue_uniqueLock.unlock();
        eventHandler->join();
        delete eventHandler;
        delete currentState;
        delete previousState;
    }

    void start() {
        eventHandler = new std::thread([&]() {
            running = true;

            while (running) {
                std::unique_lock<std::mutex> stateMachineEventQueue_uniqueLock(
                    eventQueue_mutex);

                while (eventQueue.empty() && running) {
                    eventQueue_ready.wait(stateMachineEventQueue_uniqueLock);
                    if (eventQueue.empty() && running) {
                        L_DEBUG(connection->owner->ID, "Spurious wakeup");
                    }
                }
                if (running) {
                    Event event = eventQueue.front();
                    eventQueue.pop();

                    L_DEBUG(connection->owner->ID,
                            "Passing event " + getEventName(event) +
                                " to the current state (" + currentState->NAME +
                                ")");
                    State* hanglingState_forlogs =
                        currentState;  // only used in the logs
                    bool result = currentState && currentState->onEvent(event);
                    L_DEBUG(connection->owner->ID,
                            "Event " + getEventName(event) +
                                (result ? " handled" : " NOT handled") +
                                " by " + hanglingState_forlogs->NAME);
                } else {
                    L_VERBOSE(connection->owner->ID,
                              "Shutting down state machine: " +
                                  connection->owner->ID);
                }

                stateMachineEventQueue_uniqueLock.unlock();
            }
        });
    }

    /**
     * Change the current state of the Finite State Machine
     * @param newState the newly created state that becomes the new current
     * state of the Finite State Machine
     */
    void changeState(State* newState) {
        assert(newState);

        delete previousState;
        previousState = currentState;
        if (currentState == nullptr) {
            L_VERBOSE(connection->owner->ID,
                      "Initial state: " + newState->NAME);
        } else {
            L_VERBOSE(connection->owner->ID,
                      "State change: " + currentState->NAME + " -> " +
                          newState->NAME);
        }


        currentState = newState;
    }

    /**
     * Enqueue the event in the Finite state Machine event queue. It is called
     * from BGPConnection::enqueue event
     * @warning this method should be called by the BGPConnection. Use
     * BGPConnection::enqueueEvent instead
     * @param event the event triggered
     */
    void enqueueEvent(Event event) {
        std::unique_lock<std::mutex> eventQueue_uniqueLock(eventQueue_mutex);
        eventQueue.push(event);
        eventQueue_ready.notify_one();
        eventQueue_uniqueLock.unlock();
    }

    /**
     * Get the currentState of the Finite State Machine. Used for recreating the
     * flow of the FSM
     * @return the currentState value
     */
    State* getCurrentState() const { return currentState; }
};
#endif