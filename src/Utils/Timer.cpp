#include "Timer.h"

Timer :: Timer(std::string const name):NAME(std::move(name)){
    exitSignal = false;
    lockedByUser = false;
    // lock();
}

Timer :: ~Timer(){

    if(lockedByUser){
        unlock();
    }
}

void Timer :: lock(){

    mutex.lock();
    lockedByUser = true;
}

void Timer :: unlock(){
    lockedByUser = false;
    mutex.unlock();
}

void Timer :: start(const std::chrono::seconds & interval,BGPStateMachine* stateMachine, Event event){

    if(!lockedByUser){
        lock();
        timerThread = new std::thread([this, interval, stateMachine, event](){
            auto start = std::chrono::steady_clock::now();
            std::cout << "Started "<< NAME << " of "<< interval.count() <<" seconds..."<< std::endl;
            if(this->mutex.try_lock_for(interval)){
                this->mutex.unlock();
            }
            if(!this->exitSignal){
                stateMachine->handleEvent(event);
                // stateMachine->*callback(event);
            }

            auto end = std::chrono::steady_clock::now();
            std::cout << "Ended "<< NAME <<" after " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() 
                      << "s. Locked = "<< lockedByUser << std::endl;
        
        });
    }else{
        std::cout << "Timer already started!" << std::endl;
    }
}

void Timer :: stop(){

    if(lockedByUser){
        exitSignal = true;
        unlock();
    }
}

void Timer :: join(){
    if(timerThread != nullptr){
        timerThread->join();
        delete timerThread;
    }else{
        std::cout << "Timer not started" << std::endl;
    }
}


