#include "TCPStateListen.h"

#include "../../logger/Logger.h"
#include "../TCPFlag.h"
#include "TCPStateClosed.h"
#include "TCPStateSYNReceived.h"

TCPStateListen::TCPStateListen(TCPStateMachine *stateMachine)
    : TCPState(stateMachine) {
    NAME = "LISTEN";
    L_DEBUG("State created: " + NAME);
}
bool TCPStateListen::onEvent(TCPEvent event) {
    std::stack<pcpp::Layer *> *layers   = nullptr;
    pcpp::TcpLayer *           tcpLayer = nullptr;

    bool handled = true;
    switch (event) {
        case TCPEvent::ReceiveClientSYN_SendSYNACK:
            // The server device receives a SYN from a client. It sends back a
            // message that contains its own SYN and also acknowledges the one
            // it received. The server moves to the SYN-RECEIVED state.

            layers   = new std::stack<pcpp::Layer *>();
            tcpLayer = craftTCPLayer(stateMachine->connection->srcPort,
                                     stateMachine->connection->dstPort,
                                     SYN + ACK);
            layers->push(tcpLayer);
            stateMachine->connection->owner->sendPacket(
                layers, stateMachine->connection->dstAddr->toString());
            delete layers;
            stateMachine->changeState(new TCPStateSYNReceived(stateMachine));
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