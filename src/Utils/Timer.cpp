#include "Timer.h"

bool Timer::isRunning(){

    auto status = runningFuture.wait_for(std::chrono::milliseconds(0));

    if(status == std::future_status::ready){
        return false;
    }else{
        return true;
    }
}

// void Timer :: stop(){

//     timerThread
// }