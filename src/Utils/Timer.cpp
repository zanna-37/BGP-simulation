#include "Timer.h"

Timer :: Timer(){

}

Timer :: ~Timer(){

    stop();
}

void Timer :: start(std::chrono::seconds const & interval,
                    std::function<void(void)> const & callEvent){

    stop();

    {
        auto locked = std::unique_lock<std::mutex>(mutex);
        stopCondition = false;
    }

    timerThread = std::thread([=](){
        auto locked = std::unique_lock<std::mutex>(mutex);

        while(!stopCondition){
            auto result = terminate.wait_for(locked,interval);

            if(result == std::cv_status::timeout){
                callEvent();
            }
        }
    });

    timerThread.detach();
}


void Timer :: stop(){
    {
        auto locked = std::unique_lock<std::mutex>(mutex);
        stopCondition = true;
    }
    terminate.notify_one();
    
    // if (timerThread.joinable()){
    //     timerThread.join();
    // }
    
}