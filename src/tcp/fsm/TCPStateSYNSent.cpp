#include "TCPStateSYNSent.h"

#include "../../logger/Logger.h"
#include "TCPStateEnstablished.h"
#include "TCPStateSYNReceived.h"


TCPStateSYNSent::TCPStateSYNSent(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    NAME = "SYN-SENT";
    L_DEBUG("State created: " + NAME);
}

bool TCPStateSYNSent::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case TCPEvent::ReceiveSYN_SendACK:
            // If the device that has sent its SYN message receives a SYN from
            // the other device but not an ACK for its own SYN, it acknowledges
            // the SYN it receives and then transitions to SYN-RECEIVED to wait
            // for the acknowledgment to its SYN.
            stateMachine->changeState(new TCPStateSYNReceived(stateMachine));


            break;
        case TCPEvent::ReceiveSYNACKSendACK:
            // If the device that sent the SYN receives both an acknowledgment
            // to its SYN and also a SYN from the other device, it acknowledges
            // the SYN received and then moves straight to the ESTABLISHED
            // state.
            stateMachine->changeState(new TCPStateEnstablished(stateMachine));

            break;

        default:
            handled = false;
            break;
    }
    return handled;
}