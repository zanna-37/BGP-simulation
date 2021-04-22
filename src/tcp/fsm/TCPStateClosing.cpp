#include "TCPStateClosing.h"

#include "../../logger/Logger.h"
#include "TCPStateClosed.h"
#include "TCPStateTimeWait.h"

TCPStateClosing::TCPStateClosing(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    name = "CLOSING";
    L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
            "State created: " + name);
}

bool TCPStateClosing::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case TCPEvent::ReceiveACKforFIN:
            // The device receives an acknowledgment for its close request. It
            // transitions to the TIME-WAIT state.
            stateMachine->timeWaitTimer->start();
            stateMachine->changeState(new TCPStateTimeWait(stateMachine));
            break;
        case TCPEvent::ReceiveRST:
            stateMachine->changeState(new TCPStateClosed(stateMachine));
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}