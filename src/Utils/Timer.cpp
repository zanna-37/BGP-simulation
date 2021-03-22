#include "Timer.h"

Timer :: Timer( std::string const name, 
                Event timerExpires, 
                BGPStateMachine* stateMachine,
                std::chrono::seconds interval):
                NAME(std::move(name)), 
                timerExpires(timerExpires), 
                stateMachine(stateMachine),
                interval(interval){
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

void Timer :: start(){

    if(!lockedByUser){
        lock();
        timerThread = new std::thread([&](){
            auto start = std::chrono::steady_clock::now();
            std::cout << "Started "<< NAME << " of "<< interval.count() <<" seconds..."<< std::endl;
            if(mutex.try_lock_for(interval)){
                mutex.unlock();
            }
            if(!exitSignal){
                    stateMachine->handleEvent(timerExpires);
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


