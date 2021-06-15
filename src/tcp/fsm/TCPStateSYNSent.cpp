#include "TCPStateSYNSent.h"

#include <atomic>
#include <condition_variable>
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
#include "TCPStateEstablished.h"
#include "TCPStateMachine.h"
#include "TCPStateSYNReceived.h"
#include "TcpLayer.h"


TCPStateSYNSent::TCPStateSYNSent(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    name = "SYN-SENT";
    // L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
    // "State created: " + name);
}

bool TCPStateSYNSent::onEvent(TCPEvent event) {
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
            stateMachine->enqueueEvent(TCPEvent::Send);
            break;

        case TCPEvent::Receive:
            handled = false;  // TODO implement
            break;

        case TCPEvent::Close:
            // Delete the TCB and return "error: closing" responses to any
            // queued SENDs, or RECEIVEs.
            stateMachine->connection->running = false;
            stateMachine->connection->tryingToEstablish_mutex.lock();
            stateMachine->connection->tryingToEstablish = false;
            stateMachine->connection->tryingToEstablish_mutex.unlock();
            stateMachine->connection->tryingToEstablish_wakeup.notify_all();
            stateMachine->changeState(new TCPStateClosed(stateMachine));
            break;

        case TCPEvent::Abort:
            // All queued SENDs and RECEIVEs should be given "connection reset"
            // notification, delete the TCB, enter CLOSED state, and return.
            stateMachine->connection->running = false;
            stateMachine->connection->tryingToEstablish_mutex.lock();
            stateMachine->connection->tryingToEstablish = false;
            stateMachine->connection->tryingToEstablish_mutex.unlock();
            stateMachine->connection->tryingToEstablish_wakeup.notify_all();
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

            // first check the ACK bit. If the ACK bit is set: if SEG.ACK =<
            // ISS, or SEG.ACK > SND.NXT, send a reset (unless the RST bit is
            // set, if so drop the segment and return) <SEQ=SEG.ACK><CTL=RST>
            // and discard the segment. Return.
            // If SND.UNA =< SEG.ACK =< SND.NXT then the ACK is acceptable.
            // <-- Not implemented

            if (isFlag8Set(receivedFlags, TCPFlag::RST)) {
                // If the RST bit is set.
                // If the ACK was acceptable then signal the user "error:
                // connection reset"
                // <-- Not implemented

                // drop the segment, enter CLOSED state, delete TCB, and return.
                stateMachine->connection->running = false;
                stateMachine->connection->tryingToEstablish_mutex.lock();
                stateMachine->connection->tryingToEstablish = false;
                stateMachine->connection->tryingToEstablish_mutex.unlock();
                stateMachine->connection->tryingToEstablish_wakeup.notify_all();
                stateMachine->changeState(new TCPStateClosed(stateMachine));

                // Otherwise (no ACK) drop the segment and return.
                // <-- Not implemented
            }
            // third check the security and precedence. If the
            // security/compartment in the segment does not exactly match the
            // security/compartment in the TCB, send a reset
            // If there is an ACK <SEQ=SEG.ACK><CTL=RST> Otherwise
            // <SEQ=0><ACK=SEG.SEQ+SEG.LEN><CTL=RST,ACK> If there is an ACK the
            // precedence in the segment must match the precedence in the TCB,
            // if not, send a reset <SEQ=SEG.ACK><CTL=RST> If there is no ACK.
            // If the precedence in the segment is higher than the precedence in
            // the TCB then if allowed by the user and the system raise the
            // precedence in the TCB to that in the segment, if not allowed to
            // raise the prec then send a reset.
            // <SEQ=0><ACK=SEG.SEQ+SEG.LEN><CTL=RST,ACK> If the precedence in
            // the segment is lower than the precedence in the TCB continue. If
            // a reset was sent, discard the segment and return.
            // <-- Not implemented
            else if (isFlag8Set(receivedFlags, TCPFlag::SYN)) {
                // fourth check the SYN bit. This step should be reached only if
                // the ACK is ok, or there is no ACK, and it the segment did not
                // contain a RST.

                // If the SYN bit is on and the security/compartment and
                // precedence are acceptable then, RCV.NXT is set to SEG.SEQ+1,
                // IRS is set to SEG.SEQ. SND.UNA should be advanced to equal
                // SEG.ACK (if there is an ACK), and any segments on the
                // retransmission queue which are thereby acknowledged should be
                // removed.
                // <-- Not implemented

                if (isFlag8Set(receivedFlags, TCPFlag::ACK)) {
                    // If SND.UNA > ISS (our SYN has been ACKed), change the
                    // connection state to ESTABLISHED, form an ACK segment
                    // <SEQ=SND.NXT><ACK=RCV.NXT><CTL=ACK> and send it.
                    stateMachine->changeState(
                        new TCPStateEstablished(stateMachine));
                    stateMachine->connection->tryingToEstablish_mutex.lock();
                    stateMachine->connection->tryingToEstablish = false;
                    stateMachine->connection->tryingToEstablish_mutex.unlock();
                    stateMachine->connection->tryingToEstablish_wakeup
                        .notify_one();
                    stateMachine->connection->sendAckToPeer();
                } else {
                    // Data or controls which were queued for transmission may
                    // be included. If there are other controls or text in the
                    // segment then continue processing at the sixth step below
                    // where the URG bit is checked, otherwise return.
                    // <-- Not implemented

                    // Otherwise enter SYN-RECEIVED, form a SYN,ACK segment
                    // <SEQ=ISS><ACK=RCV.NXT><CTL=SYN,ACK>and send it. If there
                    // are other controls or text in the segment, queue them for
                    // processing after the ESTABLISHED state has been reached,
                    // return.
                    stateMachine->changeState(
                        new TCPStateSYNReceived(stateMachine));
                    stateMachine->connection->sendSynAckToPeer();
                }
            } else {
                // fifth, if neither of the SYN or RST bits is set then drop the
                // segment and return.
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
