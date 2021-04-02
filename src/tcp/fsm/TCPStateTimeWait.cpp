#include "TCPStateTimeWait.h"


bool TCPStateTimeWait::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case TCPEvent::TimerExpiration:
            // After a designated wait period, device transitions to the CLOSED
            // state.
            stateMachine->changeState(new TCPStateClosed(stateMachine));
            break;
        default:
            handled = false;
            break;
    }
    return handled;
}