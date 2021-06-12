#include "TCPStateClosed.h"

#include <atomic>
#include <memory>
#include <stack>
#include <string>
#include <utility>

#include "../../entities/Device.h"
#include "../../logger/Logger.h"
#include "../TCPConnection.h"
#include "../TCPEvent.h"
#include "Layer.h"
#include "TCPStateListen.h"
#include "TCPStateMachine.h"
#include "TCPStateSYNSent.h"


TCPStateClosed::TCPStateClosed(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    name = "CLOSED";
    // L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
    // "State created: " + name);
}

bool TCPStateClosed::onEvent(TCPEvent event) {
    bool handled = true;

    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> segment;
    switch (event) {
        case TCPEvent::OpenPassive:
            // A server begins the process of connection setup by doing a
            // passive open on a TCP port. At the same time, it sets up the data
            // structure (transmission control block or TCB) needed to manage
            // the connection. It then transitions to the LISTEN state.
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Listening on port " +
                        std::to_string(stateMachine->connection->srcPort));

            stateMachine->connection->running = true;
            stateMachine->changeState(new TCPStateListen(stateMachine));
            break;

        case TCPEvent::OpenActive:
            // A client begins connection setup by sending a SYN message, and
            // also sets up a TCB for this connection. It then transitions to
            // the SYN-SENT state.
            stateMachine->connection->running = true;
            stateMachine->connection->sendSyn();
            stateMachine->changeState(new TCPStateSYNSent(stateMachine));
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
            L_ERROR(stateMachine->connection->owner->ID,
                    "connection does not exist");
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
