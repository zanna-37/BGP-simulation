#include "TCPStateClosing.h"

bool TCPStateClosing::onEvent(TCPEvent event) {
    bool handled;
    switch (event) {
        case ReceiveACKforFIN:
            // The device receives an acknowledgment for its close request. It
            // transitions to the TIME-WAIT state.
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}