#include "TCPStateEnstablished.h"

#include <IPLayer.h>
#include <TcpLayer.h>

#include <stack>

#include "../../logger/Logger.h"
#include "../TCPFlag.h"
#include "TCPStateCloseWait.h"
#include "TCPStateClosed.h"
#include "TCPStateFINWait1.h"

TCPStateEnstablished::TCPStateEnstablished(TCPStateMachine *stateMachine)
    : TCPState(stateMachine) {
    name = "ENSTABLISHED";
    // L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
    // "State created: " + name);
}

bool TCPStateEnstablished::onEvent(TCPEvent event) {
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
            // Queue this until all preceding SENDs have been segmentized, then
            // form a FIN segment and send it. In any case, enter FIN-WAIT-1
            // state.
            stateMachine->connection->sendFin();
            stateMachine->changeState(new TCPStateFINWait1(stateMachine));
            break;

        case TCPEvent::Abort:
            // Send a reset segment: <SEQ=SND.NXT><CTL=RST>
            stateMachine->connection->sendRst();
            // TODO All queued SENDs and RECEIVEs should be given "connection
            // reset" notification;
            // TODO all segments queued for transmission (except for the RST
            // formed above) or retransmission should be flushed,

            // delete the TCB, enter CLOSED state, and return.
            stateMachine->connection->running = false;
            stateMachine->changeState(new TCPStateClosed(stateMachine));
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
