#include "TCPStateClosing.h"

#include "../../logger/Logger.h"
#include "TCPStateTimeWait.h"

TCPStateClosing::TCPStateClosing(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    NAME = "CLOSING";
    L_DEBUG("State created: " + NAME);
}

bool TCPStateClosing::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case TCPEvent::ReceiveACKforFIN:
            // The device receives an acknowledgment for its close request. It
            // transitions to the TIME-WAIT state.

            stateMachine->changeState(new TCPStateTimeWait(stateMachine));
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}