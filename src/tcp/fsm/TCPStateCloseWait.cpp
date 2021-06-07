#include "TCPStateCloseWait.h"

#include <stack>

#include "../../logger/Logger.h"
#include "../TCPFlag.h"
#include "TCPStateClosed.h"
#include "TCPStateClosing.h"
#include "TCPStateLastACK.h"
#include "TcpLayer.h"

TCPStateCloseWait::TCPStateCloseWait(TCPStateMachine *stateMachine)
    : TCPState(stateMachine) {
    name = "CLOSE_WAIT";
    // L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
    // "State created: " + name);
}

bool TCPStateCloseWait::onEvent(TCPEvent event) {
    bool handled = true;

    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> segment;
    switch (event) {
        case TCPEvent::OpenPassive:
            handled = false;  // TODO implement
            break;

        case TCPEvent::OpenActive:
            handled = false;  // TODO implement
            break;

        case TCPEvent::Send:
            handled = false;  // TODO implement
            break;

        case TCPEvent::Receive:
            handled = false;  // TODO implement
            break;

        case TCPEvent::Close:
            // Queue this request until all preceding SENDs have been
            // segmentized; then send a FIN segment, enter CLOSING state.
            stateMachine->connection->sendFin();
            // TODO check: at page 60 they say to switch to state closing, but
            // at page 22 the diagram suggest to go to last-ack state.
            stateMachine->changeState(new TCPStateLastACK(stateMachine));
            break;

        case TCPEvent::Abort:
            handled = false;  // TODO implement
            break;

        case TCPEvent::Status:
            handled = false;  // TODO implement
            break;

        case TCPEvent::SegmentArrives:
            segment = std::move(stateMachine->connection->getNextSegment());
            handled = false;  // TODO implement
            break;

        case TCPEvent::UserTimeout:
            handled = false;  // TODO implement
            break;

        case TCPEvent::RetransmissionTimeout:
            handled = false;  // TODO implement
            break;

        case TCPEvent::TimeWaitTimeout:
            // If the time-wait timeout expires on a connection delete the TCB,
            // enter the CLOSED state and return.
            stateMachine->connection->running = false;
            stateMachine->changeState(new TCPStateClosed(stateMachine));
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}
