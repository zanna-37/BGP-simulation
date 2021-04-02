#include "TCPStateEnstablished.h"


bool TCPStateEnstablished::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case TCPEvent::CloseSendFIN:
            // A device can close the connection by sending a message with the
            // FIN (finish) bit sent and transition to the FIN-WAIT-1 state.
            stateMachine->changeState(new TCPStateFINWait1(stateMachine));
            break;
        case TCPEvent::ReceiveFIN:
            // A device may receive a FIN message from its connection partner
            // asking that the connection be closed. It will acknowledge this
            // message and transition to the CLOSE-WAIT stat
            stateMachine->changeState(new TCPStateCloseWait(stateMachine));

            break;

        default:
            handled = false;
            break;
    }
    return handled;
}