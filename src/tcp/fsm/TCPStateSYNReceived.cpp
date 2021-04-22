#include "TCPStateSYNReceived.h"

#include "../../logger/Logger.h"
#include "../TCPFlag.h"
#include "TCPStateClosed.h"
#include "TCPStateEnstablished.h"


TCPStateSYNReceived::TCPStateSYNReceived(TCPStateMachine *stateMachine)
    : TCPState(stateMachine) {
    name = "SYN-RECEIVED";
    L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
            "State created: " + name);
}

bool TCPStateSYNReceived::onEvent(TCPEvent event) {
    std::stack<pcpp::Layer *> *layers   = nullptr;
    pcpp::TcpLayer *           tcpLayer = nullptr;

    bool handled = true;
    switch (event) {
        case TCPEvent::ReceiveACK:
            // When the device receives the ACK to the SYN it sent, it
            // transitions to the ESTABLISHED state.
            stateMachine->changeState(new TCPStateEnstablished(stateMachine));

            break;
        case TCPEvent::SendRST:
            layers   = new std::stack<pcpp::Layer *>();
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