#include "TCPStateEnstablished.h"

#include <IPLayer.h>
#include <TcpLayer.h>

#include <stack>

#include "../../logger/Logger.h"
#include "../TCPFlag.h"
#include "TCPStateCloseWait.h"
#include "TCPStateClosed.h"
#include "TCPStateFINWait1.h"

TCPStateEnstablished::TCPStateEnstablished(TCPStateMachine *stateMachine)
    : TCPState(stateMachine) {
    NAME = "ENSTABLISHED";
    L_DEBUG("State created: " + NAME);
}

bool TCPStateEnstablished::onEvent(TCPEvent event) {
    stack<pcpp::Layer *> *layers   = nullptr;
    pcpp::TcpLayer *      tcpLayer = nullptr;

    bool handled = true;

    switch (event) {
        case TCPEvent::CloseSendFIN:
            // A device can close the connection by sending a message with the
            // FIN (finish) bit sent and transition to the FIN-WAIT-1 state.

            layers   = new std::stack<pcpp::Layer *>();
            tcpLayer = craftTCPLayer(stateMachine->connection->srcPort,
                                     stateMachine->connection->dstPort,
                                     FIN);
            layers->push(tcpLayer);
            stateMachine->connection->owner->sendPacket(
                layers, stateMachine->connection->dstAddr->toString());
            delete layers;
            stateMachine->changeState(new TCPStateFINWait1(stateMachine));
            break;
        case TCPEvent::ReceiveFINSendACK:
            // A device may receive a FIN message from its connection partner
            // asking that the connection be closed. It will acknowledge this
            // message and transition to the CLOSE-WAIT state

            layers   = new std::stack<pcpp::Layer *>();
            tcpLayer = craftTCPLayer(stateMachine->connection->srcPort,
                                     stateMachine->connection->dstPort,
                                     FIN + ACK);
            layers->push(tcpLayer);
            stateMachine->connection->owner->sendPacket(
                layers, stateMachine->connection->dstAddr->toString());
            delete layers;
            stateMachine->changeState(new TCPStateCloseWait(stateMachine));

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