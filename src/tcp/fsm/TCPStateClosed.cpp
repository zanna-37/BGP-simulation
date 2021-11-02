#include "TCPStateClosed.h"

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
#include "TCPStateListen.h"
#include "TCPStateMachine.h"
#include "TCPStateSYNSent.h"
#include "TcpLayer.h"


TCPStateClosed::TCPStateClosed(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    name = "CLOSED";
    // L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
    // "State created: " + name);
}

bool TCPStateClosed::onEvent(TCPEvent event) {
    bool handled = true;

    switch (event) {
        case TCPEvent::OpenPassive:
            // A server begins the process of connection setup by doing a
            // passive open on a TCP port. At the same time, it sets up the data
            // structure (transmission control block or TCB) needed to manage
            // the connection. It then transitions to the LISTEN state.
            L_DEBUG(
                stateMachine->connection->owner->ID + " " + stateMachine->name,
                "Listening on port " +
                    stateMachine->connection->srcAddr.toString() + ":" +
                    std::to_string(stateMachine->connection->srcPort));

            stateMachine->connection->running = true;
            stateMachine->changeState(new TCPStateListen(stateMachine));
            break;

        case TCPEvent::OpenActive:
            // A client begins connection setup by sending a SYN message, and
            // also sets up a TCB for this connection. It then transitions to
            // the SYN-SENT state.
            stateMachine->connection->running = true;
            stateMachine->connection->tryingToEstablish_mutex.lock();
            stateMachine->connection->tryingToEstablish = true;
            stateMachine->connection->tryingToEstablish_mutex.unlock();
            stateMachine->connection->sendSynToPeer();
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
            L_ERROR(
                stateMachine->connection->owner->ID + " " + stateMachine->name,
                "Connection does not exist (aborting closed connection)");
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

            if (isFlag8Set(receivedFlags, TCPFlag::RST)) {
            } else if (isFlag8Set(receivedFlags, TCPFlag::ACK)) {
            } else if (isFlag8Set(receivedFlags, TCPFlag::SYN)) {
            } else {
            }
            handled = false;  // TODO implement
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
            stateMachine->changeState(new TCPStateClosed(stateMachine));
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}
