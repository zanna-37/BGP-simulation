#include "TCPStateSYNSent.h"

#include <stack>

#include "../../entities/Device.h"
#include "../../logger/Logger.h"
#include "../TCPFlag.h"
#include "TCPStateClosed.h"
#include "TCPStateEnstablished.h"
#include "TCPStateSYNReceived.h"


TCPStateSYNSent::TCPStateSYNSent(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    name = "SYN-SENT";
    // L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
    // "State created: " + name);
}

bool TCPStateSYNSent::onEvent(TCPEvent event) {
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
            //  Delete the TCB and return "error:  closing" responses to any
            //  queued SENDs, or RECEIVEs.
            stateMachine->connection->running = false;
            stateMachine->connection->established_wakeup.notify_all();
            stateMachine->changeState(new TCPStateClosed(stateMachine));
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
