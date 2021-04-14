#include "TCPStateMachine.h"


TCPStateMachine::TCPStateMachine(TCPConnection* connection)
    : StateMachine(connection) {
    initializeTimers();
}

TCPStateMachine::~TCPStateMachine() { delete timeWaitTimer; }


void TCPStateMachine::initializeTimers() { resetTimeWaitTimer(); }

void TCPStateMachine::resetTimeWaitTimer() {
    if (timeWaitTimer != nullptr) {
        timeWaitTimer->stop();
        delete timeWaitTimer;
    }

    timeWaitTimer = new TCPTimer(
        "TimeWaitTimer", this, TCPEvent::TimerExpiration, timeWaitTime);
}