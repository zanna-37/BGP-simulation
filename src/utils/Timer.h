#ifndef TIMER_H
#define TIMER_H

#include <functional>
#include <chrono>
#include <future>
#include <cstdio>
#include <future>
#include <mutex>
#include <iostream>
#include <cstring>

#include "../bgp/Event.h"
#include "../bgp/fsm/BGPStateMachine.h"
#include "../logger/Logger.h"


// http://coliru.stacked-crooked.com/a/98fdcd78c99e948c

class Timer {

private:
    std::timed_mutex mutex;
    std::atomic<bool> lockedByUser;
    std::thread* timerThread = nullptr;
    std::atomic_bool exitSignal;
    const std::string NAME;
    Event timerExpires;
    BGPStateMachine* stateMachine;
    std::chrono::seconds interval;
    bool running;
    std::chrono::milliseconds remainingTime;
    

    void lock();
    void unlock();

public:
    Timer(std::string const name, Event timerExpires, BGPStateMachine* stateMachine, std::chrono::seconds interval);
    ~Timer();

    void start();

    void stop();
    void join();

    bool isRunning() const { return running; }

    std::chrono::milliseconds getRemainingTime() const { return remainingTime; }
    void setRemainingTime(const std::chrono::milliseconds &value) { remainingTime = value; }
    
};

#endif