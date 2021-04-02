#include "Timer.h"

Timer ::Timer(std::string                                         name,
              BGPStateMachine<BGPConnection, BGPState, BGPEvent>* stateMachine,
              BGPEvent             eventToSendUponExpire,
              std::chrono::seconds totalDuration)
    : NAME(std::move(name)),
      stateMachine(stateMachine),
      eventToSendUponExpire(eventToSendUponExpire),
      totalDuration(totalDuration),
      duration(std::chrono::milliseconds::min()) {}

Timer ::~Timer() {
    if (timerThread != nullptr) {
        L_ERROR("Timer " + NAME +
                " has not been stopped before deletion.\nCall stop() before "
                "deleting the Timer.");
        stop();
    }
}

void Timer ::start() {
    mutex.lock();

    if (timerState != UNINITIALIZED) {
        L_ERROR("Illegal state: timer already started.");
    } else {
        timerState = TICKING;
        sleepMutex.lock();

        timerThread = new std::thread([&]() {
            auto start = std::chrono::steady_clock::now();

            std::chrono::milliseconds timeToSleep;

            if (duration.count() < 0) {
                timeToSleep = totalDuration;
                L_DEBUG("START " + NAME + ": " +
                        to_string(timeToSleep.count()) + "ms");
            } else {
                timeToSleep = duration;
                L_DEBUG("RESUME " + NAME + ": " +
                        to_string(timeToSleep.count()) + "ms");
            }

            sleepMutex.try_lock_for(timeToSleep);

            mutex.lock();
            if (timerState == TICKING) {
                L_DEBUG("TIMEOUT " + NAME + ": performing actions");
                timerState = EXECUTING_SCHEDULED_TASK;
                stateMachine->enqueueEvent(eventToSendUponExpire);
                timerState = COMPLETED;

            } else {
                L_DEBUG(NAME + ": " + "was stopped, skipping actions");
            }

            auto end = std::chrono::steady_clock::now();

            L_DEBUG(
                "END " + NAME + " after " +
                to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                              end - start)
                              .count()) +
                "ms");
            mutex.unlock();
        });
    }
    mutex.unlock();
}

// void Timer ::pause() {
//    remainingTime =
//        std::chrono::duration_cast<std::chrono::milliseconds>(
//            interval -
//            std::chrono::duration_cast<std::chrono::milliseconds>(
//                end - start));
//}

void Timer ::stop() {
    mutex.lock();
    switch (timerState) {
        case UNINITIALIZED:
            L_DEBUG("STOP " + NAME + ": state uninitialized");
            break;
        case TICKING:
            timerState = CANCELLED;
            sleepMutex.unlock();
            L_DEBUG("STOP " + NAME + ": state ticking");
            L_DEBUG("CANCEL " + NAME);
            break;
        case EXECUTING_SCHEDULED_TASK:
        case COMPLETED:
            L_DEBUG("STOP " + NAME + ": state expired");
            break;
        case CANCELLED:
            L_DEBUG("STOP " + NAME + ": state already cancelled");
            break;
    }
    mutex.unlock();

    if (timerThread != nullptr) {
        timerThread->join();
        delete timerThread;
        timerThread = nullptr;
    }
}
