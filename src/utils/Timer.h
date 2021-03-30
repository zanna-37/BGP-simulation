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
    std::chrono::milliseconds remainingDurationAfterPause;

    std::mutex       mutex;
    std::timed_mutex sleepMutex;
    TimerState       timerState = UNINITIALIZED;


   public:
    Timer(std::string          name,
          BGPStateMachine*     stateMachine,
          Event                eventToSendUponExpire,
          std::chrono::seconds totalDuration);
    ~Timer();

    /**
     * Start the timer thread setting the duration to the default valued defined in the constructor
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
     * Get the remaining duration of the timer
     * @return the remaining time
     */
    std::chrono::milliseconds getRemainingTime() const {
        return remainingDurationAfterPause;
    }

    /**
     * Set the remaining duration after a timer pause
     * @param value the remaining duration
     */
    void setRemainingTime(const std::chrono::milliseconds& value) {
        remainingDurationAfterPause = value;
    }
};

#endif
