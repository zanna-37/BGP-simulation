#include "TCPStateFINWait2.h"

#include "../../logger/Logger.h"
#include "TCPStateTimeWait.h"

TCPStateFINWait2::TCPStateFINWait2(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    NAME = "FIN-WAIT-2";
    L_DEBUG("State created: " + NAME);
}

bool TCPStateFINWait2::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case TCPEvent::ReceiveFINSendACK:
            // The device receives a FIN from the other device. It acknowledges
            // it and moves to the TIME-WAIT state.
            stateMachine->changeState(new TCPStateTimeWait(stateMachine));

            break;

        default:
            handled = false;
            break;
    }
    return handled;
}