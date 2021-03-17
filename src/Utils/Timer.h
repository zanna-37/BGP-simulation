#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <ctime>
#include <cmath>


// check with threads: https://stackoverflow.com/questions/21521282/basic-timer-with-stdthread-and-stdchrono
class Timer {

public:
    
    void start();
    void stop();
    double elapsedMilliseconds();
    double elapsedSeconds();

private:
    std::chrono::time_point<std::chrono::system_clock> startTime;
    std::chrono::time_point<std::chrono::system_clock> endTime;
    bool running = false;

    
};

#endif