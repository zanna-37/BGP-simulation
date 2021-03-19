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


// http://coliru.stacked-crooked.com/a/98fdcd78c99e948c

class Timer {

private:
    std::timed_mutex mutex;
    std::atomic<bool> lockedByUser;
    std::thread* timerThread = nullptr;
    std::atomic_bool exitSignal;
    const std::string NAME;
    

    void lock();
    void unlock();

public:
    Timer(std::string const name);
    ~Timer();

    void start(const std::chrono::seconds & interval, std::function<void(void)> const & callback);

    void stop();
    void join();

    
};

#endif