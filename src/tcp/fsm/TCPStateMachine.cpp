#include "TCPStateMachine.h"

#include "../../entities/Device.h"
#include "../../fsm/StateMachine.h"
#include "../TCPConnection.h"
#include "../TCPEvent.h"
#include "../TCPTimer.h"
#include "IpAddress.h"
#include "TCPState.h"
#include "TCPStateClosed.h"


TCPStateMachine::TCPStateMachine(TCPConnection* connection)
    : StateMachine(connection, "TCPfsm") {
    initializeTimers();
    changeState(new TCPStateClosed(this));
}

TCPStateMachine::~TCPStateMachine() {
    if (timeWaitTimer != nullptr) {
        timeWaitTimer->stop();
        delete timeWaitTimer;
    }
}


void TCPStateMachine::initializeTimers() { resetTimeWaitTimer(); }

void TCPStateMachine::resetTimeWaitTimer() {
    if (timeWaitTimer != nullptr) {
        timeWaitTimer->stop();
        delete timeWaitTimer;
    }

    timeWaitTimer = new TCPTimer(
        "TimeWaitTimer", this, TCPEvent::TimeWaitTimeout, timeWaitTime);
}

std::string TCPStateMachine::toString() {
    if (connection) {
        return connection->srcAddr.toString() + ":" +
               std::to_string(connection->srcPort) + " " +
               connection->dstAddr.toString() + ":" +
               std::to_string(connection->dstPort);
    } else {
        return "";
    }
}
