#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <cstdio>
#include <cstring>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>

#include "../bgp/BGPConnection.h"
#include "../bgp/BGPEvent.h"
#include "../bgp/fsm/BGPStateMachine.h"
#include "../logger/Logger.h"


// http://coliru.stacked-crooked.com/a/98fdcd78c99e948c

enum TimerState {
    /**
     * The timer has been initialized but never started.
     */
    UNINITIALIZED,

    /**
     * The timer is counting down waiting to expire. Ticking refers to the
     * "tic-tac" that a timer does when counting down.
     */
    TICKING,

    /**
     * The timer is expired without a cancellation, therefore it is executing
     * the action that has been scheduled to be done after the expiration.
     */
    EXECUTING_SCHEDULED_TASK,

    /**
     * The timer is expired and has completed the scheduled actions.
     */
    COMPLETED,

    /**
     * The timer has been cancelled before expiration, therefore the actions
     * will not be performed.
     */
    CANCELLED
};

template <class Connection, class State, class Event>
class BGPStateMachine;
class BGPConnection;
class BGPState;
// forward declaration
class Timer {
   private:
    const std::string                                   NAME;
    std::thread*                                        timerThread = nullptr;
    BGPStateMachine<BGPConnection, BGPState, BGPEvent>* stateMachine;
    BGPEvent                                            eventToSendUponExpire;
    std::chrono::seconds                                totalDuration;
    std::chrono::milliseconds                           duration;

    std::mutex       mutex;
    std::timed_mutex sleepMutex;
    TimerState       timerState = UNINITIALIZED;


   public:
    /**
     * Timer constructor
     * @param name the string name of the timer
     * @param stateMachine a pointer to the owner state machine
     * @param eventToSendUponExpire the name of the event to send upon the
     * expiration of the timer
     * @param totalDuration the initial total duration of the timer. Usually it
     * is set to the default value that can be found inside the state machine
     */
    Timer(std::string                                         name,
          BGPStateMachine<BGPConnection, BGPState, BGPEvent>* stateMachine,
          BGPEvent             eventToSendUponExpire,
          std::chrono::seconds totalDuration);
    ~Timer();

    /**
     * Start the timer thread setting the duration to the value defined
     * in the constructor.
     * This can only be called once.
     */
    void start();

    /**
     * Stop the timer and join the timer thread.
     * It is safe to call it multiple times and in every state.
     */
    void stop();

    /**
     * Get the current state of the timer.
     *
     * @return the timer state value
     */
    TimerState getState() const { return timerState; }

    /**
     * Return the remaining time after a pause. It differs from \a totalDuration
     * because \a totalDuration is the initial value of the timer,
     * while \a duration is uninitialized when the timer starts for the first
     * time and it is only used if the user calls \a Timer::pause (// TODO
     * currently not implemented).
     *
     * @return The duration value.
     */
    std::chrono::milliseconds getDuration() const { return duration; }
};

#endif
