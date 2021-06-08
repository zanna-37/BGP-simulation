#include "TCPStateSYNReceived.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <stack>
#include <string>
#include <tuple>
#include <utility>

#include "../../entities/Device.h"
#include "../../logger/Logger.h"
#include "../../utils/Bits.h"
#include "../TCPConnection.h"
#include "../TCPEvent.h"
#include "../TCPFlag.h"
#include "IpAddress.h"
#include "Layer.h"
#include "TCPStateCloseWait.h"
#include "TCPStateClosed.h"
#include "TCPStateEnstablished.h"
#include "TCPStateFINWait1.h"
#include "TCPStateMachine.h"
#include "TcpLayer.h"


TCPStateSYNReceived::TCPStateSYNReceived(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    name = "SYN-RECEIVED";
    // L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
    // "State created: " + name);
}

bool TCPStateSYNReceived::onEvent(TCPEvent event) {
    bool handled = true;

    switch (event) {
        case TCPEvent::OpenPassive:
            handled = false;  // TODO implement
            break;

        case TCPEvent::OpenActive:
            handled = false;  // TODO implement
            break;

        case TCPEvent::Send:
            // Queue the data for transmission after entering ESTABLISHED state.
            // If no space to queue, respond with "error:  insufficient
            // resources".

            // TODO this is terribly inefficient, because it just waits some
            // time and then the event will probably be caught by the same state
            // (this) until we change to the right one (Established).
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            L_DEBUG(
                stateMachine->connection->owner->ID + " " + stateMachine->name,
                "Rescheduling " + getEventName(TCPEvent::Send) + "...");
            stateMachine->enqueueEvent(TCPEvent::Send);
            break;

        case TCPEvent::Receive:
            handled = false;  // TODO implement
            break;

        case TCPEvent::Close:
            // TODO If no SENDs have been issued and there is no pending data to
            // send

            // then form a FIN segment and send it
            stateMachine->connection->sendFinToPeer();
            // enter FIN-WAIT-1 state;
            stateMachine->changeState(new TCPStateFINWait1(stateMachine));

            // TODO otherwise queue for processing after entering ESTABLISHED
            // state.
            break;

        case TCPEvent::Abort:
            // Send a reset segment: <SEQ=SND.NXT><CTL=RST>
            stateMachine->connection->sendRstToPeer();
            // All queued SENDs and RECEIVEs should be given "connection reset"
            // notification; all segments queued for transmission (except for
            // the RST formed above) or retransmission should be flushed, delete
            // the TCB, enter CLOSED state, and return.
            stateMachine->connection->running = false;
            stateMachine->connection->tryingToEstablish_mutex.lock();
            stateMachine->connection->tryingToEstablish = false;
            stateMachine->connection->tryingToEstablish_mutex.unlock();
            stateMachine->connection->tryingToEstablish_wakeup.notify_one();
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
                segmentPair = stateMachine->connection->getNextSegment();

            std::tie(fromAddr, segment) = std::move(segmentPair);

            auto receivedTcpLayer = std::move(segment->top());
            segment->pop();

            auto* receivedTcpLayer_weak =
                dynamic_cast<pcpp::TcpLayer*>(receivedTcpLayer.get());

            uint8_t receivedFlags = TCPConnection::parseTCPFlags(
                receivedTcpLayer_weak->getTcpHeader());

            // First check sequence number
            // Segments are processed in sequence.  Initial tests on arrival are
            // used to discard old duplicates, but further processing is done in
            // SEG.SEQ order. If a segment's contents straddle the boundary
            // between old and new, only the new parts should be processed.
            // There are four cases for the acceptability test for an incoming
            // segment: OMISSIS
            // <-- Not implemented

            if (isFlag8Set(receivedFlags, TCPFlag::RST)) {
                // If this connection was initiated with a passive OPEN (i.e.,
                // came from the LISTEN state), then return this connection to
                // LISTEN state and return.  The user need not be informed.
                // <-- Not implemented

                // If this connection was initiated with an active OPEN (i.e.,
                // came from SYN-SENT state) then the connection was refused,
                // signal the user "connection refused". In either case, all
                // segments on the retransmission queue should be removed. And
                // in the active OPEN case, enter the CLOSED state and delete
                // the TCB, and return.

                stateMachine->connection->running = false;
                stateMachine->connection->tryingToEstablish_mutex.lock();
                stateMachine->connection->tryingToEstablish = false;
                stateMachine->connection->tryingToEstablish_mutex.unlock();
                stateMachine->connection->tryingToEstablish_wakeup.notify_one();
                stateMachine->changeState(new TCPStateClosed(stateMachine));
            }
            // third check security and precedence
            // If the security/compartment and precedence in the segment do not
            // exactly match the security/compartment and precedence in the TCB
            // then send a reset, and return.

            else if (isFlag8Set(receivedFlags, TCPFlag::SYN)) {
                // If the SYN is in the window it is an error, send a reset, any
                // outstanding RECEIVEs and SEND should receive
                // "reset" responses, all segment queues should be flushed,
                // the user should also receive an unsolicited general
                // "connection reset" signal, enter the CLOSED state, delete the
                // TCB, and return.
                stateMachine->connection->sendRstToPeer();
                stateMachine->connection->running = false;
                stateMachine->connection->tryingToEstablish_mutex.lock();
                stateMachine->connection->tryingToEstablish = false;
                stateMachine->connection->tryingToEstablish_mutex.unlock();
                stateMachine->connection->tryingToEstablish_wakeup.notify_one();
                stateMachine->changeState(new TCPStateClosed(stateMachine));

                // If the SYN is not in the window this step would not be
                // reached and an ack would have been sent in the first step
                // (sequence number check).
            } else if (isFlag8Set(receivedFlags, TCPFlag::ACK)) {
                // fifth check the ACK field,
                // if the ACK bit is off drop the segment and return
                // If SND.UNA =< SEG.ACK =< SND.NXT then enter ESTABLISHED state
                // and continue processing.
                stateMachine->changeState(
                    new TCPStateEnstablished(stateMachine));
                stateMachine->connection->tryingToEstablish_mutex.lock();
                stateMachine->connection->tryingToEstablish = false;
                stateMachine->connection->tryingToEstablish_mutex.unlock();
                stateMachine->connection->tryingToEstablish_wakeup.notify_one();

                // If the segment acknowledgment is not acceptable, form a reset
                // segment, <SEQ=SEG.ACK><CTL=RST> and send it.
                // <-- Not implemented
            } else if (isFlag8Set(receivedFlags, TCPFlag::FIN)) {
                // Enter the CLOSE-WAIT state.
                stateMachine->connection->tryingToEstablish_mutex.lock();
                stateMachine->connection->tryingToEstablish = false;
                stateMachine->connection->tryingToEstablish_mutex.unlock();
                stateMachine->connection->tryingToEstablish_wakeup.notify_one();
                stateMachine->changeState(new TCPStateCloseWait(stateMachine));
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
            stateMachine->connection->tryingToEstablish_mutex.lock();
            stateMachine->connection->tryingToEstablish = false;
            stateMachine->connection->tryingToEstablish_mutex.unlock();
            stateMachine->connection->tryingToEstablish_wakeup.notify_one();
            stateMachine->changeState(new TCPStateClosed(stateMachine));
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}
