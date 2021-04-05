#include "TCPStateFINWait1.h"

#include "../../logger/Logger.h"
#include "TCPStateClosing.h"
#include "TCPStateFINWait2.h"

TCPStateFINWait1::TCPStateFINWait1(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    NAME = "FIN-WAIT-1";
    L_DEBUG("State created: " + NAME);
}
bool TCPStateFINWait1::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case TCPEvent::ReceiveACKforFIN:
            // The device receives an acknowledgment for its close request. It
            // transitions to the FIN-WAIT-2 state.
            stateMachine->changeState(new TCPStateFINWait2(stateMachine));

            break;
        case TCPEvent::ReceiveFINSendACK:
            // The device does not receive an ACK for its own FIN, but receives
            // a FIN from the other device. It acknowledges it, and moves to the
            // CLOSING state.
            stateMachine->changeState(new TCPStateClosing(stateMachine));

            break;

        default:
            handled = false;
            break;
    }
    return handled;
}