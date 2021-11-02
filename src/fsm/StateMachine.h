#ifndef BGPSIMULATION_FSM_STATEMACHINE_H
#define BGPSIMULATION_FSM_STATEMACHINE_H

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
                        L_DEBUG_CONN(connection->owner->ID,
                                     connection->toString(),
                                     "Spurious wakeup");
                    }
                }
                if (running) {
                    Event event = std::move(eventQueue.front());
                    eventQueue.pop();
                    stateMachineEventQueue_uniqueLock.unlock();

                    std::string eventName = getEventName(event);
                    L_DEBUG_CONN(connection->owner->ID + " " + name,
                                 connection->toString(),
                                 "Passing event " + eventName +
                                     " to the current state (" +
                                     currentState->name + ") " + toString());

                    State* hanglingState_forlogs =
                        currentState;  // only used in the logs
                    bool result =
                        currentState && currentState->onEvent(std::move(event));

                    {
                        std::string owner_str =
                            connection->owner->ID + " " + name;
                        std::string message_str =
                            "Event " + eventName +
                            (result ? " handled" : " NOT handled") + " by " +
                            hanglingState_forlogs->name + " " + toString();

                        if (result) {
                            // L_DEBUG_CONN(owner_str,connection->toString(),
                            // message_str);
                        } else {
                            L_WARNING_CONN(
                                owner_str, connection->toString(), message_str);
                        }
                    }
                } else {
                    L_VERBOSE_CONN(connection->owner->ID + " " + name,
                                   connection->toString(),
                                   "Shutting down state machine");
                    stateMachineEventQueue_uniqueLock.unlock();
                }
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
            L_VERBOSE_CONN(
                connection->owner->ID + " " + name,
                connection->toString(),
                "Initial state: " + newState->name + " " + toString());
        } else {
            L_VERBOSE_CONN(connection->owner->ID + " " + name,
                           connection->toString(),
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
        eventQueue_mutex.lock();
        eventQueue.push(std::move(event));
        eventQueue_mutex.unlock();

        eventQueue_ready.notify_one();
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

#endif  // BGPSIMULATION_FSM_STATEMACHINE_H
