#include "TCPStateTimeWait.h"


bool TCPStateTimeWait::onEvent(TCPEvent event) {
    bool handled;
    switch (event) {
        case TimerExpiration:
            // After a designated wait period, device transitions to the CLOSED
            // state.

            break;

        default:
            handled = false;
            break;
    }
    return handled;
}