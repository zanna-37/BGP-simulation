#include "TCPStateTimeWait.h"

#include "../../logger/Logger.h"
#include "TCPStateClosed.h"

TCPStateTimeWait::TCPStateTimeWait(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    NAME = "TIME-WAIT";
    L_DEBUG("State created: " + NAME);
}
bool TCPStateTimeWait::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case TCPEvent::TimerExpiration:
            // After a designated wait period, device transitions to the CLOSED
            // state.
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