#ifndef TIMER_H
#define TIMER_H

#include <functional>
#include <chrono>
#include <future>
#include <cstdio>
#include <future>


// check with threads: https://stackoverflow.com/questions/21521282/basic-timer-with-stdthread-and-stdchrono
// using namespace std::chrono_literals;
class Timer {

public:
    std::thread timerThread;
    // std::function<void()> triggerEvent;
    template <typename F>
    Timer(int sleepTime, F triggerEvent) /*: triggerEvent(triggerEvent)*/{
        // //unknown code
        // std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(f), std::forward<arguments(args)...));

        runningFuture = runningPromise.get_future();
        timerThread = std::thread([sleepTime, triggerEvent, this](){
            std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
            this->runningPromise.set_value(true);
            // p.set_value(true);
            triggerEvent();
            
        });
        // timerThread.join();
        timerThread.detach();

    }

    bool isRunning();
    void stop();

private:
    
    std::promise<bool> runningPromise;
    std::future<bool> runningFuture;
    
};

#endif