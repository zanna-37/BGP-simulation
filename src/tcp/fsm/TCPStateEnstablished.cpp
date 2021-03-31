#include "TCPStateEnstablished.h"


bool TCPStateEnstablished::onEvent(TCPEvent event) {
    bool handled;
    switch (event) {
        case CloseSendFIN:
            // A device can close the connection by sending a message with the
            // FIN (finish) bit sent and transition to the FIN-WAIT-1 state.
            break;
        case ReceiveFIN:
            // A device may receive a FIN message from its connection partner
            // asking that the connection be closed. It will acknowledge this
            // message and transition to the CLOSE-WAIT stat
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}