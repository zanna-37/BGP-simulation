#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <queue>
#include <thread>

#include "../logger/Logger.h"

template <typename Connection, typename State, typename Event>
class StateMachine {
   public:
    Connection* connection    = nullptr;
    State*      previousState = nullptr;
    State*      currentState  = nullptr;
    std::string name;

    StateMachine(Connection* connection, std::string name)
        : connection(connection), name(name) {}

    ~StateMachine() {
        running = false;

        eventQueue_ready.notify_all();
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

                    L_DEBUG(connection->owner->ID + " " + name,
                            "Passing event " + getEventName(event) +
                                " to the current state (" + currentState->name +
                                ") " + toString());

                    State* hanglingState_forlogs =
                        currentState;  // only used in the logs
                    bool result = currentState && currentState->onEvent(event);

                    {
                        std::string owner_str =
                            connection->owner->ID + " " + name;
                        std::string message_str =
                            "Event " + getEventName(event) +
                            (result ? " handled" : " NOT handled") + " by " +
                            hanglingState_forlogs->name + " " + toString();

                        if (result) {
                            L_DEBUG(owner_str, message_str);
                        } else {
                            L_WARNING(owner_str, message_str);
                        }
                    }
                } else {
                    L_VERBOSE(connection->owner->ID + " " + name,
                              "Shutting down state machine");
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
            L_VERBOSE(connection->owner->ID + " " + name,
                      "Initial state: " + newState->name + " " + toString());
        } else {
            L_VERBOSE(connection->owner->ID + " " + name,
                      "State change: " + currentState->name + " -> " +
                          newState->name + " " + toString());
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

   protected:
    virtual std::string toString() = 0;

   private:
    std::thread*            eventHandler = nullptr;
    std::mutex              eventQueue_mutex;
    std::condition_variable eventQueue_ready;
    std::queue<Event>       eventQueue;
    std::atomic<bool>       running = {false};
};
#endif
