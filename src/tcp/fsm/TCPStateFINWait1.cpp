#include "TCPStateFINWait1.h"


bool TCPStateFINWait1::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case ReceiveACKforFIN:
            // The device receives an acknowledgment for its close request. It
            // transitions to the FIN-WAIT-2 state.
            stateMachine->changeState(new TCPStateFINWait2(stateMachine));

            break;
        case ReceiveFINSendACK:
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