#include "TCPStateLastACK.h"

#include "../../logger/Logger.h"
#include "TCPStateClosed.h"

TCPStateLastACK::TCPStateLastACK(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    name = "LAST-ACK";
    // L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
    // "State created: " + name);
}

bool TCPStateLastACK::onEvent(TCPEvent event) {
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
            handled = false;  // TODO implement
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
