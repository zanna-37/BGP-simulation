#include "TCPStateSYNSent.h"

#include <stack>

#include "../../logger/Logger.h"
#include "../TCPFlag.h"
#include "TCPStateClosed.h"
#include "TCPStateEnstablished.h"
#include "TCPStateSYNReceived.h"


TCPStateSYNSent::TCPStateSYNSent(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    name = "SYN-SENT";
    L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
            "State created: " + name);
}

bool TCPStateSYNSent::onEvent(TCPEvent event) {
    std::stack<pcpp::Layer*>* layers   = nullptr;
    pcpp::TcpLayer*           tcpLayer = nullptr;

    bool handled = true;
    switch (event) {
        case TCPEvent::ReceiveSYN_SendACK:
            // If the device that has sent its SYN message receives a SYN from
            // the other device but not an ACK for its own SYN, it acknowledges
            // the SYN it receives and then transitions to SYN-RECEIVED to wait
            // for the acknowledgment to its SYN.
            layers   = new std::stack<pcpp::Layer*>();
            tcpLayer = craftTCPLayer(stateMachine->connection->srcPort,
                                     stateMachine->connection->dstPort,
                                     ACK);
            layers->push(tcpLayer);
            stateMachine->connection->owner->sendPacket(
                layers, stateMachine->connection->dstAddr->toString());
            delete layers;
            stateMachine->changeState(new TCPStateSYNReceived(stateMachine));


            break;
        case TCPEvent::ReceiveSYNACKSendACK:
            // If the device that sent the SYN receives both an acknowledgment
            // to its SYN and also a SYN from the other device, it acknowledges
            // the SYN received and then moves straight to the ESTABLISHED
            // state.

            layers   = new std::stack<pcpp::Layer*>();
            tcpLayer = craftTCPLayer(stateMachine->connection->srcPort,
                                     stateMachine->connection->dstPort,
                                     ACK);
            layers->push(tcpLayer);
            stateMachine->connection->owner->sendPacket(
                layers, stateMachine->connection->dstAddr->toString());
            delete layers;
            stateMachine->changeState(new TCPStateEnstablished(stateMachine));

            break;
        case TCPEvent::SendRST:
            layers   = new std::stack<pcpp::Layer*>();
            tcpLayer = craftTCPLayer(stateMachine->connection->srcPort,
                                     stateMachine->connection->dstPort,
                                     RST);
            layers->push(tcpLayer);
            stateMachine->connection->owner->sendPacket(
                layers, stateMachine->connection->dstAddr->toString());
            delete layers;
            stateMachine->changeState(new TCPStateClosed(stateMachine));
            break;
        case TCPEvent::ReceiveRST:
            stateMachine->changeState(new TCPStateClosed(stateMachine));
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}