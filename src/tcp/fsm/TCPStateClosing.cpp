#include "TCPStateClosing.h"

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