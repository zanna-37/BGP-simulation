#include "TCPStateClosed.h"


bool TCPStateClosed::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case PassiveOpen:
            // A server begins the process of connection setup by doing a
            // passive open on a TCP port. At the same time, it sets up the data
            // structure (transmission control block or TCB) needed to manage
            // the connection. It then transitions to the LISTEN state.

            stateMachine->changeState(new TCPStateListen(stateMachine));

            break;
        case ActiveOpen_SendSYN:
            // A client begins connection setup by sending a SYN message, and
            // also sets up a TCB for this connection. It then transitions to
            // the SYN-SENT state.

            stateMachine->changeState(new TCPStateSYNSent(stateMachine));

        default:
            handled = false;
            break;
    }
    return handled;
}