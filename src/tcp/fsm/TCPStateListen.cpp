#include "TCPStateListen.h"

#include "../../logger/Logger.h"
#include "TCPStateSYNReceived.h"

TCPStateListen::TCPStateListen(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    NAME = "LISTEN";
    L_DEBUG("State created: " + NAME);
}
bool TCPStateListen::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case TCPEvent::ReceiveClientSYN_SendSYNACK:
            // The server device receives a SYN from a client. It sends back a
            // message that contains its own SYN and also acknowledges the one
            // it received. The server moves to the SYN-RECEIVED state.
            stateMachine->changeState(new TCPStateSYNReceived(stateMachine));


            break;

        default:
            handled = false;
            break;
    }
    return handled;
}