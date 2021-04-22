#include "TCPStateTimeWait.h"

#include "../../logger/Logger.h"
#include "TCPStateClosed.h"

TCPStateTimeWait::TCPStateTimeWait(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    name = "TIME-WAIT";
    L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
            "State created: " + name);
}
bool TCPStateTimeWait::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case TCPEvent::TimerExpiration:
            // After a designated wait period, device transitions to the CLOSED
            // state.
            stateMachine->changeState(new TCPStateClosed(stateMachine));
            // stateMachine->connection->owner->removeTCPConnection(
            //     stateMachine->connection);
            break;
        case TCPEvent::ReceiveRST:
            stateMachine->changeState(new TCPStateClosed(stateMachine));
            break;
        default:
            handled = false;
            break;
    }
    return handled;
}