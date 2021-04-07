#include "TCPStateCloseWait.h"

#include <TcpLayer.h>

#include <stack>

#include "../../logger/Logger.h"
#include "../TCPFlag.h"
#include "TCPStateClosed.h"
#include "TCPStateLastACK.h"

TCPStateCloseWait::TCPStateCloseWait(TCPStateMachine *stateMachine)
    : TCPState(stateMachine) {
    NAME = "CLOSE_WAIT";
    L_DEBUG("State created: " + NAME);
}

bool TCPStateCloseWait::onEvent(TCPEvent event) {
    bool handled = true;

    stack<pcpp::Layer *> *layers   = nullptr;
    pcpp::TcpLayer *      tcpLayer = nullptr;

    switch (event) {
        case TCPEvent::CloseSendFIN:
            // The application using TCP, having been informed the other process
            // wants to shut down, sends a close request to the TCP layer on the
            // machine upon which it is running. TCP then sends a FIN to the
            // remote device that already asked to terminate the connection.
            // This device now transitions to LAST-ACK.

            layers   = new std::stack<pcpp::Layer *>();
            tcpLayer = craftTCPLayer(stateMachine->connection->srcPort,
                                     stateMachine->connection->dstPort,
                                     FIN);
            layers->push(tcpLayer);
            stateMachine->connection->owner->sendPacket(
                layers, stateMachine->connection->dstAddr->toString());

            stateMachine->changeState(new TCPStateLastACK(stateMachine));
            break;
        case TCPEvent::SendRST:
            layers   = new std::stack<pcpp::Layer *>();
            tcpLayer = craftTCPLayer(stateMachine->connection->srcPort,
                                     stateMachine->connection->dstPort,
                                     RST);
            layers->push(tcpLayer);
            stateMachine->connection->owner->sendPacket(
                layers, stateMachine->connection->dstAddr->toString());
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
