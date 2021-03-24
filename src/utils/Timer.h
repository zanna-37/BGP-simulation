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

    void start();

    void stop();

    TimerState getState() const { return timerState; }

    std::chrono::milliseconds getRemainingTime() const {
        return remainingDurationAfterPause;
    }
    void setRemainingTime(const std::chrono::milliseconds& value) {
        remainingDurationAfterPause = value;
    }
};

#endif
