#include "TCPStateLastACK.h"


bool TCPStateLastACK::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case TCPEvent::ReceiveACKforFIN:
            // The device receives an acknowledgment for its close request. We
            // have now sent our FIN and had it acknowledged, and received the
            // other device's FIN and acknowledged it, so we go straight to the
            // CLOSED state.
            stateMachine->changeState(new TCPStateClosed(stateMachine));

            break;

        default:
            handled = false;
            break;
    }
    return handled;
}