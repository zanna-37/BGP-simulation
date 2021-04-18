#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <cstdio>
#include <cstring>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>

#include "../logger/Logger.h"

enum TimerState {
    /**
     * The timer has been initialized but never started.
     */
    UNINITIALIZED,

    /**
     * The timer is counting down waiting to expire. Ticking refers to the
     * "tic-tac" that a timer does when counting down.
     */
    TICKING,

    /**
     * The timer is expired without a cancellation, therefore it is executing
     * the action that has been scheduled to be done after the expiration.
     */
    EXECUTING_SCHEDULED_TASK,

    /**
     * The timer is expired and has completed the scheduled actions.
     */
    COMPLETED,

    /**
     * The timer has been cancelled before expiration, therefore the actions
     * will not be performed.
     */
    CANCELLED
};

// http://coliru.stacked-crooked.com/a/98fdcd78c99e948c
template <class StateMachine, class Event>
class Timer {
   private:
    const std::string         name;
    std::thread*              timerThread = nullptr;
    StateMachine*             stateMachine;
    Event                     eventToSendUponExpire;
    std::chrono::seconds      totalDuration;
    std::chrono::milliseconds duration;

    std::mutex       mutex;
    std::timed_mutex sleepMutex;
    TimerState       timerState = UNINITIALIZED;


   public:
    /**
     * Timer constructor
     * @param name the string name of the timer
     * @param stateMachine a pointer to the owner state machine
     * @param eventToSendUponExpire the name of the event to send upon the
     * expiration of the timer
     * @param totalDuration the initial total duration of the timer. Usually it
     * is set to the default value that can be found inside the state machine
     */
    Timer(std::string          name,
          StateMachine*        stateMachine,
          Event                eventToSendUponExpire,
          std::chrono::seconds totalDuration)
        : name(std::move(name)),
          stateMachine(stateMachine),
          eventToSendUponExpire(eventToSendUponExpire),
          totalDuration(totalDuration),
          duration(std::chrono::milliseconds::min()) {}
    ~Timer() {
        if (timerThread != nullptr) {
            L_ERROR(stateMachine->connection->owner->ID,
                    "Timer " + name +
                        " has not been stopped before deletion.\nCall stop() "
                        "before "
                        "deleting the Timer.");
            stop();
        }
    }

    /**
     * Start the timer thread setting the duration to the value defined
     * in the constructor.
     * This can only be called once.
     */
    void start() {
        mutex.lock();

        if (timerState != UNINITIALIZED) {
            L_ERROR(stateMachine->connection->owner->ID,
                    "Illegal state: timer already started.");
        } else {
            timerState = TICKING;
            sleepMutex.lock();

            timerThread = new std::thread([&]() {
                auto start = std::chrono::steady_clock::now();

                std::chrono::milliseconds timeToSleep;

                if (duration.count() < 0) {
                    timeToSleep = totalDuration;
                    L_DEBUG(stateMachine->connection->owner->ID,
                            "START " + name + ": " +
                                to_string(timeToSleep.count()) + "ms");
                } else {
                    timeToSleep = duration;
                    L_DEBUG(stateMachine->connection->owner->ID,
                            "RESUME " + name + ": " +
                                to_string(timeToSleep.count()) + "ms");
                }

                sleepMutex.try_lock_for(timeToSleep);

                mutex.lock();
                if (timerState == TICKING) {
                    L_DEBUG(stateMachine->connection->owner->ID,
                            "TIMEOUT " + name + ": performing actions");
                    timerState = EXECUTING_SCHEDULED_TASK;
                    stateMachine->enqueueEvent(eventToSendUponExpire);
                    timerState = COMPLETED;

                } else {
                    L_DEBUG(stateMachine->connection->owner->ID,
                            name + ": " + "was stopped, skipping actions");
                }

                auto end = std::chrono::steady_clock::now();

                L_DEBUG(
                    stateMachine->connection->owner->ID,
                    "END " + name + " after " +
                        to_string(std::chrono::duration_cast<
                                      std::chrono::milliseconds>(end - start)
                                      .count()) +
                        "ms");
                mutex.unlock();
            });
        }
        mutex.unlock();
    }

    // void pause() {
    //    remainingTime =
    //        std::chrono::duration_cast<std::chrono::milliseconds>(
    //            interval -
    //            std::chrono::duration_cast<std::chrono::milliseconds>(
    //                end - start));
    //}

    /**
     * Stop the timer and join the timer thread.
     * It is safe to call it multiple times and in every state.
     */
    void stop() {
        mutex.lock();
        switch (timerState) {
            case UNINITIALIZED:
                L_DEBUG(stateMachine->connection->owner->ID,
                        "STOP " + name + ": state uninitialized");
                break;
            case TICKING:
                timerState = CANCELLED;
                sleepMutex.unlock();
                L_DEBUG(stateMachine->connection->owner->ID,
                        "STOP " + name + ": state ticking");
                L_DEBUG(stateMachine->connection->owner->ID, "CANCEL " + name);
                break;
            case EXECUTING_SCHEDULED_TASK:
            case COMPLETED:
                L_DEBUG(stateMachine->connection->owner->ID,
                        "STOP " + name + ": state expired");
                break;
            case CANCELLED:
                L_DEBUG(stateMachine->connection->owner->ID,
                        "STOP " + name + ": state already cancelled");
                break;
        }
        mutex.unlock();

        if (timerThread != nullptr) {
            timerThread->join();
            delete timerThread;
            timerThread = nullptr;
        }
    }

    /**
     * Get the current state of the timer.
     *
     * @return the timer state value
     */
    TimerState getState() const { return timerState; }

    /**
     * Return the remaining time after a pause. It differs from \a
     * totalDuration because \a totalDuration is the initial value
     * of the timer, while \a duration is uninitialized when the
     * timer starts for the first time and it is only used if the
     * user calls \a Timer::pause
     * (// TODO currently not implemented).
     *
     * @return The duration value.
     */
    std::chrono::milliseconds getDuration() const { return duration; }
};

#endif