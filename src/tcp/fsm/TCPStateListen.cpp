#include "TCPStateListen.h"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
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
#include "TCPStateClosed.h"
#include "TCPStateMachine.h"
#include "TCPStateSYNReceived.h"
#include "TcpLayer.h"


TCPStateListen::TCPStateListen(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    name = "LISTEN";
    // L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
    // "State created: " + name);
}
bool TCPStateListen::onEvent(TCPEvent event) {
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
            // Any outstanding RECEIVEs are returned with "error:  closing"
            // responses.  Delete TCB, enter CLOSED state, and return.
            stateMachine->connection->running = false;
            stateMachine->connection->pendingConnections_wakeup.notify_one();
            stateMachine->changeState(new TCPStateClosed(stateMachine));
            break;

        case TCPEvent::Abort:
            // Any outstanding RECEIVEs should be returned with "error:
            // connection reset" responses. Delete TCB, enter CLOSED state, and
            // return.
            stateMachine->connection->running = false;
            stateMachine->connection->pendingConnections_wakeup.notify_one();
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

            if (isFlag8Set(receivedFlags, TCPFlag::RST)) {
                // first check for an RST, an incoming RST should be ignored.
                // Return.
            } else if (isFlag8Set(receivedFlags, TCPFlag::ACK)) {
                // second check for an ACK. Any acknowledgment is bad if it
                // arrives on a connection still in the LISTEN state. An
                // acceptable reset segment should be formed for any arriving
                // ACK-bearing segment. The RST should be formatted as follows:
                // <SEQ=SEG.ACK><CTL=RST>. Return.
                stateMachine->connection->sendRstTo(
                    fromAddr, receivedTcpLayer_weak->getSrcPort());
            } else if (isFlag8Set(receivedFlags, TCPFlag::SYN)) {
                // third check for a SYN If the SYN bit is set, check the
                // security. If the security/compartment on the incoming segment
                // does not exactly match the security/compartment in the TCB
                // then send a reset and return. <SEQ=SEG.ACK><CTL=RST>
                // <-- Not implemented

                // If the SEG.PRC is greater than the TCB.PRC then if allowed by
                // the user and the system set TCB.PRC<-SEG.PRC, if not allowed
                // send a reset and return. <SEQ=SEG.ACK><CTL=RST>
                // <-- Not implemented

                // If the SEG.PRC is less than the TCB.PRC then continue. Set
                // RCV.NXT to SEG.SEQ+1, IRS is set to SEG.SEQ and any other
                // control or text should be queued for processing later.  ISS
                // should be selected
                // <-- Not implemented

                // and a SYN segment sent of the form:
                // <SEQ=ISS><ACK=RCV.NXT><CTL=SYN,ACK>
                std::shared_ptr<TCPConnection> newTcpConnectedConnection =
                    stateMachine->connection
                        ->createConnectedConnectionFromListening(
                            fromAddr, receivedTcpLayer_weak->getSrcPort());

                std::unique_lock<std::mutex> pendingConnections_uniqueLock(
                    stateMachine->connection->pendingConnections_mutex);
                stateMachine->connection->pendingConnections.push(
                    newTcpConnectedConnection);
                stateMachine->connection->pendingConnections_wakeup
                    .notify_one();

                newTcpConnectedConnection->sendSynAckToPeer();

                // SND.NXT is set to ISS+1 and SND.UNA to ISS.
                // <-- Not implemented
                // The connection state should be changed to SYN-RECEIVED.
                stateMachine->connection->tryingToEstablish_mutex.lock();
                stateMachine->connection->tryingToEstablish = true;
                stateMachine->connection->tryingToEstablish_mutex.unlock();
                newTcpConnectedConnection->stateMachine->changeState(
                    new TCPStateSYNReceived(
                        newTcpConnectedConnection->stateMachine));
                stateMachine->connection->owner->bind(
                    newTcpConnectedConnection);

                // Note that any other incoming control or data (combined with
                // SYN) will be processed in the SYN-RECEIVED state, but
                // processing of SYN and ACK should not be repeated. If the
                // listen was not fully specified (i.e., the foreign socket was
                // not fully specified), then the unspecified fields should be
                // filled in now.
                // <-- Not implemented
            } else {
                // fourth other text or control.
                // Any other control or text - bearing segment(not containing
                // SYN) must have an ACK and thus would be discarded by the ACK
                // processing.
                stateMachine->connection->sendAckToPeer();
                // An incoming RST segment could not be valid, since it could
                // not have been sent in response to anything sent by this
                // incarnation of the connection. So you are unlikely to get
                // here, but if you do, drop the segment, and return.
                // <-- Not implemented
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
