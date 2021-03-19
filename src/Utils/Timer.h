#ifndef TIMER_H
#define TIMER_H

#include <functional>
#include <chrono>
#include <future>
#include <cstdio>
#include <future>
#include <mutex>

//https://stackoverflow.com/questions/32693103/threaded-timer-interrupting-a-sleep-stopping-it

class Timer {

public:
    Timer();

    ~Timer();

    void start(std::chrono::seconds const & interval, std::function<void(void)> const & callEvent);
    void stop();
private:
    bool stopCondition;
    std::thread timerThread;
    std::mutex mutex;
    std::condition_variable terminate;
};

#endif