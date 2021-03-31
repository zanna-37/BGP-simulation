#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <cstdio>
#include <cstring>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>

#include "../bgp/Event.h"
#include "../bgp/fsm/BGPStateMachine.h"
#include "../logger/Logger.h"


// http://coliru.stacked-crooked.com/a/98fdcd78c99e948c

enum TimerState {
    UNINITIALIZED,
    TICKING,
    EXECUTING_SCHEDULED_TASK,
    COMPLETED,
    CANCELLED
};

class BGPStateMachine;  // forward declaration
class Timer {
   private:
    const std::string         NAME;
    std::thread*              timerThread = nullptr;
    BGPStateMachine*          stateMachine;
    Event                     eventToSendUponExpire;
    std::chrono::seconds      totalDuration;
    std::chrono::milliseconds duration;

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
    Timer(std::string          name,
          BGPStateMachine*     stateMachine,
          Event                eventToSendUponExpire,
          std::chrono::seconds totalDuration);
    ~Timer();

    /**
     * Start the timer thread setting the duration to the default value defined
     * in the constructor
     */
    void start();

    /**
     * Stop the timer and join the timer thread
     */
    void stop();

    /**
     * Get the actual state of the timer
     * @return the timer state value
     */
    TimerState getState() const { return timerState; }

    /**
     * return the remaining time after a pause.It differs from total duration
     * because total duration is the initial default value of the timer,
     *  while duration is uninitialized when the timer starts for the first
     * time and it is used only if the user call Timer::pause (// TODO)
     * @return the duration value
     */
    std::chrono::milliseconds getDuration() const { return duration; }
};

#endif
