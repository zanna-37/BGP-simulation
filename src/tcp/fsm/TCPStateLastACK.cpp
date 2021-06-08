#include "TCPStateLastACK.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <stack>
#include <string>
#include <utility>

#include "../../entities/Device.h"
#include "../../logger/Logger.h"
#include "../../utils/Bits.h"
#include "../TCPConnection.h"
#include "../TCPEvent.h"
#include "../TCPFlag.h"
#include "IpAddress.h"
#include "Layer.h"
#include "TCPStateClosed.h"
#include "TCPStateMachine.h"
#include "TcpLayer.h"


TCPStateLastACK::TCPStateLastACK(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    name = "LAST-ACK";
    // L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
    // "State created: " + name);
}

bool TCPStateLastACK::onEvent(TCPEvent event) {
    bool handled = true;

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
            // Respond with "ok" and delete the TCB, enter CLOSED state, and
            // return.
            stateMachine->connection->running = false;
            stateMachine->changeState(new TCPStateClosed(stateMachine));
            break;

        case TCPEvent::Status:
            handled = false;  // TODO implement
            break;

        case TCPEvent::SegmentArrives: {
            std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> segment;
            pcpp::IPv4Address                                         fromAddr;

            std::pair<pcpp::IPv4Address,
                      std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>>
                segmentPair =
                    std::move(stateMachine->connection->getNextSegment());

            fromAddr = segmentPair.first;
            segment  = std::move(segmentPair.second);

            auto receivedTcpLayer = std::move(segment->top());
            segment->pop();

            auto* receivedTcpLayer_weak =
                dynamic_cast<pcpp::TcpLayer*>(receivedTcpLayer.get());

            uint8_t receivedFlags = TCPConnection::parseTCPFlags(
                receivedTcpLayer_weak->getTcpHeader());

            // First check sequence number
            // Segments are processed in sequence.  Initial tests on arrival
            // are used to discard old duplicates, but further processing is
            // done in SEG.SEQ order. If a segment's contents straddle the
            // boundary between old and new, only the new parts should be
            // processed. There are four cases for the acceptability test
            // for an incoming segment: OMISSIS
            // <-- Not implemented

            if (isFlag8Set(receivedFlags, TCPFlag::RST)) {
                // If the RST bit is set then, enter the CLOSED state, delete
                // the TCB, and return.
                stateMachine->connection->running = false;
                stateMachine->connection->appReceivingQueue_wakeup.notify_one();
                stateMachine->changeState(new TCPStateClosed(stateMachine));

            } else if (isFlag8Set(receivedFlags, TCPFlag::SYN)) {
                // If the SYN is in the window it is an error, send a reset,
                // any outstanding RECEIVEs and SEND should receive "reset"
                // responses, all segment queues should be flushed, the user
                // should also receive an unsolicited general "connection
                // reset" signal, enter the CLOSED state, delete the TCB,
                // and return.
                stateMachine->connection->sendRstToPeer();
                stateMachine->connection->running = false;
                stateMachine->connection->appReceivingQueue_wakeup.notify_one();
                stateMachine->changeState(new TCPStateClosed(stateMachine));

                // If the SYN is not in the window this step would not be
                // reached and an ack would have been sent in the first step
                // (sequence number check).
            } else if (isFlag8Set(receivedFlags, TCPFlag::ACK)) {
                // The only thing that can arrive in this state is an
                // acknowledgment of our FIN. If our FIN is now acknowledged,
                // delete the TCB, enter the CLOSED state, and return.
                stateMachine->connection->running = false;
                stateMachine->connection->appReceivingQueue_wakeup.notify_one();
                stateMachine->changeState(new TCPStateClosed(stateMachine));

            } else {
            }
            break;
        }
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
