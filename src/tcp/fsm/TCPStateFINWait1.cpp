#include "TCPStateFINWait1.h"

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
#include "TCPStateClosing.h"
#include "TCPStateFINWait2.h"
#include "TCPStateMachine.h"
#include "TcpLayer.h"


TCPStateFINWait1::TCPStateFINWait1(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    name = "FIN-WAIT-1";
    // L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
    // "State created: " + name);
}

bool TCPStateFINWait1::onEvent(TCPEvent event) {
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
            // Send a reset segment: <SEQ=SND.NXT><CTL=RST>
            stateMachine->connection->sendRstToPeer();
            // All queued SENDs and RECEIVEs should be given "connection reset"
            // notification; all segments queued for transmission (except for
            // the RST formed above) or retransmission should be flushed, delete
            // the TCB, enter CLOSED state, and return.
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
                // If the RST bit is set then, any outstanding RECEIVEs and
                // SEND should receive "reset" responses. All segment queues
                // should be flushed. Users should also receive an
                // unsolicited general "connection reset" signal. Enter the
                // CLOSED state, delete the TCB, and return.

                stateMachine->connection->running = false;
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
                // If SND.UNA < SEG.ACK =< SND.NXT then, set SND.UNA <-
                // SEG.ACK. Any segments on the retransmission queue which
                // are thereby entirely acknowledged are removed. Users
                // should receive positive acknowledgments for buffers which
                // have been SENT and fully acknowledged (i.e., SEND buffer
                // should be returned with "ok" response). If the ACK is a
                // duplicate (SEG.ACK < SND.UNA), it can be ignored.  If the
                // ACK acks something not yet sent (SEG.ACK > SND.NXT) then
                // send an ACK, drop the segment, and return.
                // <-- Not implemented

                // If SND.UNA < SEG.ACK =< SND.NXT, the send window should
                // be updated. If (SND.WL1 < SEG.SEQ or (SND.WL1 = SEG.SEQ
                // and SND.WL2 =< SEG.ACK)), set SND.WND <- SEG.WND, set
                // SND.WL1 <- SEG.SEQ, and set SND.WL2 <- SEG.ACK.
                // <-- Not implemented

                // Note that SND.WND is an offset from SND.UNA, that SND.WL1
                // records the sequence number of the last segment used to
                // update SND.WND, and that SND.WL2 records the
                // acknowledgment number of the last segment used to update
                // SND.WND.  The check here prevents using old segments to
                // update the window.
                // <-- Not implemented

                // In addition to the processing for the ESTABLISHED state, if
                // our FIN is now acknowledged then enter FIN-WAIT-2 and
                // continue processing in that state.
                stateMachine->changeState(new TCPStateFINWait2(stateMachine));
            } else {
                // if the ACK bit is off drop the segment and return
                // <-- Not implemented
                // break;
            }

            // if (isFlag8Set(receivedFlags, TCPFlag::URG)) {
            // If the URG bit is set, RCV.UP <- max(RCV.UP,SEG.UP), and
            // signal the user that the remote side has urgent data if the
            // urgent pointer (RCV.UP) is in advance of the data consumed.
            // If the user has already been signaled (or is still in the
            // "urgent mode") for this continuous sequence of urgent data,
            // do not signal the user again.
            // }
            // <-- Not implemented

            // Once in the ESTABLISHED state, it is possible to deliver
            // segment text to user RECEIVE buffers.  Text from segments can
            // be moved into buffers until either the buffer is full or the
            // segment is empty.  If the segment empties and carries an PUSH
            // flag, then the user is informed, when the buffer is returned,
            // that a PUSH has been received. When the TCP takes
            // responsibility for delivering the data to the user it must
            // also acknowledge the receipt of the data.
            //
            // Once the TCP takes responsibility for the data it advances
            // RCV.NXT over the data accepted, and adjusts RCV.WND as
            // apporopriate to the current buffer availability.  The total
            // of RCV.NXT and RCV.WND should not be reduced.
            //
            // Please note the window management suggestions in section 3.7.
            // Send an acknowledgment of the form:
            // <SEQ=SND.NXT><ACK=RCV.NXT><CTL=ACK> This acknowledgment
            // should be piggybacked on a segment being transmitted if
            // possible without incurring undue delay.
            if (!segment->empty()) {
                L_DEBUG(stateMachine->connection->owner->ID,
                        "Received APP_PACKET from " +
                            stateMachine->connection->dstAddr.toString() + ":" +
                            std::to_string(stateMachine->connection->dstPort));
                stateMachine->connection->sendAckToPeer();
                stateMachine->connection->enqueuePacketToInbox(
                    std::move(segment));
            } else {
                L_DEBUG(stateMachine->connection->owner->ID,
                        "Received ACK (alone) from " +
                            stateMachine->connection->dstAddr.toString() + ":" +
                            std::to_string(stateMachine->connection->dstPort));
            }

            if (isFlag8Set(receivedFlags, TCPFlag::FIN)) {
                // If the FIN bit is set, signal the user "connection closing"
                // and return any pending RECEIVEs with same message, advance
                // RCV.NXT over the FIN, and send an acknowledgment for the FIN.
                // Note that FIN implies PUSH for any segment text not yet
                // delivered to the user.
                stateMachine->connection->sendAckToPeer();

                // If our FIN has been ACKed (perhaps in this segment), then
                // enter TIME-WAIT, start the time-wait timer, turn off the
                // other timers;
                // <-- Not implemented

                // otherwise enter the CLOSING state.
                stateMachine->changeState(new TCPStateClosing(stateMachine));
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
