#include "TCPStateSYNReceived.h"

#include "../../logger/Logger.h"
#include "TCPStateEnstablished.h"


TCPStateSYNReceived::TCPStateSYNReceived(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    NAME = "SYN-RECEIVED";
    L_DEBUG("State created: " + NAME);
}

bool TCPStateSYNReceived::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case TCPEvent::ReceiveACK:
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