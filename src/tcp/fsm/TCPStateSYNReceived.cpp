#include "TCPStateSYNReceived.h"


bool TCPStateSYNReceived::onEvent(TCPEvent event) {
    bool handled;
    switch (event) {
        case ReceiveACK:
            // When the device receives the ACK to the SYN it sent, it
            // transitions to the ESTABLISHED state.
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}