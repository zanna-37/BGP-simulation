#include "TCPStateLastACK.h"

#include "../../logger/Logger.h"
#include "TCPStateClosed.h"

TCPStateLastACK::TCPStateLastACK(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    NAME = "LAST-ACK";
    L_DEBUG("State created: " + NAME);
}

bool TCPStateLastACK::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case TCPEvent::ReceiveACKforFIN:
            // The device receives an acknowledgment for its close request. We
            // have now sent our FIN and had it acknowledged, and received the
            // other device's FIN and acknowledged it, so we go straight to the
            // CLOSED state.
            stateMachine->changeState(new TCPStateClosed(stateMachine));
            stateMachine->connection->owner->removeTCPConnection(
                stateMachine->connection);

            break;

        default:
            handled = false;
            break;
    }
    return handled;
}