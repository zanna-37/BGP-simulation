#include "TCPStateCloseWait.h"


bool TCPStateCloseWait::onEvent(TCPEvent event) {
    bool handled = true;
    switch (event) {
        case CloseSendFIN:
            // The application using TCP, having been informed the other process
            // wants to shut down, sends a close request to the TCP layer on the
            // machine upon which it is running. TCP then sends a FIN to the
            // remote device that already asked to terminate the connection.
            // This device now transitions to LAST-ACK.

            stateMachine->changeState(new TCPStateLastACK(stateMachine));
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}
