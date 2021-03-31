#include "TCPStateFINWait2.h"


bool TCPStateFINWait2::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case ReceiveFINSendACK:
            // The device receives a FIN from the other device. It acknowledges
            // it and moves to the TIME-WAIT state.
            stateMachine->changeState(new TCPStateTimeWait(stateMachine));

            break;

        default:
            handled = false;
            break;
    }
    return handled;
}