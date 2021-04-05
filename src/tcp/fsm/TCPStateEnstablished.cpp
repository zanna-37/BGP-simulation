#include "TCPStateEnstablished.h"

#include <IPLayer.h>
#include <TcpLayer.h>

#include <stack>

#include "../../logger/Logger.h"
#include "TCPStateCloseWait.h"
#include "TCPStateFINWait1.h"

TCPStateEnstablished::TCPStateEnstablished(TCPStateMachine *stateMachine)
    : TCPState(stateMachine) {
    NAME = "ENSTABLISHED";
    L_DEBUG("State created: " + NAME);
}

bool TCPStateEnstablished::onEvent(TCPEvent event) {
    pcpp::TcpLayer *      tcpLayer = nullptr;
    pcpp::IPv4Layer *     ipLayer  = nullptr;
    stack<pcpp::Layer *> *layers   = nullptr;

    bool handled = true;

    switch (event) {
        case TCPEvent::CloseSendFIN:
            // A device can close the connection by sending a message with the
            // FIN (finish) bit sent and transition to the FIN-WAIT-1 state.

            layers   = new stack<pcpp::Layer *>();
            tcpLayer = new pcpp::TcpLayer(stateMachine->connection->srcPort,
                                          stateMachine->connection->dstPort);
            tcpLayer->getTcpHeader()->finFlag = 1;
            ipLayer                           = new pcpp::IPv4Layer();
            ipLayer->setDstIPv4Address(stateMachine->connection->dstAddr);
            layers->push(tcpLayer);
            layers->push(ipLayer);
            stateMachine->connection->owner->sendPacket(layers);
            stateMachine->changeState(new TCPStateFINWait1(stateMachine));
            break;
        case TCPEvent::ReceiveFIN:
            // A device may receive a FIN message from its connection partner
            // asking that the connection be closed. It will acknowledge this
            // message and transition to the CLOSE-WAIT state

            layers   = new stack<pcpp::Layer *>();
            tcpLayer = new pcpp::TcpLayer(stateMachine->connection->srcPort,
                                          stateMachine->connection->dstPort);
            tcpLayer->getTcpHeader()->finFlag = 1;
            tcpLayer->getTcpHeader()->ackFlag = 1;
            ipLayer                           = new pcpp::IPv4Layer();
            ipLayer->setDstIPv4Address(stateMachine->connection->dstAddr);
            layers->push(tcpLayer);
            layers->push(ipLayer);
            stateMachine->connection->owner->sendPacket(layers);
            stateMachine->changeState(new TCPStateCloseWait(stateMachine));

            break;

        default:
            handled = false;
            break;
    }
    return handled;
}