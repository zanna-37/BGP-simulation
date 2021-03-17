#include "Timer.h"

void Timer :: start(){

    startTime = std::chrono::system_clock::now();
    running = true;
}

void Timer :: stop(){
    endTime = std::chrono::system_clock::now();
    running = false;
}

double Timer :: elapsedMilliseconds(){

    std::chrono::time_point<std::chrono::system_clock> interval;

    if(running){
        
        interval = std::chrono::system_clock::now();
    }else{

        interval = endTime;
    }

    return std::chrono::duration_cast<std::chrono::milliseconds>(interval - startTime).count();
}

double Timer :: elapsedSeconds(){

    return elapsedMilliseconds() / 1000.0;
}
