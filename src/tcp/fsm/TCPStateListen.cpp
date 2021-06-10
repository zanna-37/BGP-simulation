#include "TCPStateListen.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <stack>
#include <string>
#include <utility>

#include "../../entities/Device.h"
#include "../TCPConnection.h"
#include "../TCPEvent.h"
#include "Layer.h"
#include "TCPStateClosed.h"
#include "TCPStateMachine.h"


TCPStateListen::TCPStateListen(TCPStateMachine *stateMachine)
    : TCPState(stateMachine) {
    name = "LISTEN";
    // L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
    // "State created: " + name);
}
bool TCPStateListen::onEvent(TCPEvent event) {
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
            // Any outstanding RECEIVEs are returned with "error:  closing"
            // responses.  Delete TCB, enter CLOSED state, and return.
            stateMachine->connection->running = false;
            stateMachine->connection->pendingConnections_wakeup.notify_one();
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
