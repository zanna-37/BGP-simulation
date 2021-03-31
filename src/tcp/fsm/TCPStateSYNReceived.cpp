#include "TCPStateSYNReceived.h"


bool TCPStateSYNReceived::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case ReceiveACK:
            // When the device receives the ACK to the SYN it sent, it
            // transitions to the ESTABLISHED state.
            stateMachine->changeState(new TCPStateEnstablished(stateMachine));

            break;

        default:
            handled = false;
            break;
    }
    return handled;
}