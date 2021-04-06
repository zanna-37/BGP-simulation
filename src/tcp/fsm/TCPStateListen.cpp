#include "TCPStateListen.h"

#include <IPLayer.h>
#include <TcpLayer.h>

#include <stack>

#include "../../logger/Logger.h"
#include "TCPStateSYNReceived.h"

TCPStateListen::TCPStateListen(TCPStateMachine *stateMachine)
    : TCPState(stateMachine) {
    NAME = "LISTEN";
    L_DEBUG("State created: " + NAME);
}
bool TCPStateListen::onEvent(TCPEvent event) {
    pcpp::TcpLayer *      tcpLayer = nullptr;
    pcpp::IPv4Layer *     ipLayer  = nullptr;
    stack<pcpp::Layer *> *layers   = nullptr;

    bool handled = true;
    switch (event) {
        case TCPEvent::ReceiveClientSYN_SendSYNACK:
            // The server device receives a SYN from a client. It sends back a
            // message that contains its own SYN and also acknowledges the one
            // it received. The server moves to the SYN-RECEIVED state.
            layers   = new stack<pcpp::Layer *>();
            tcpLayer = new pcpp::TcpLayer(stateMachine->connection->srcPort,
                                          stateMachine->connection->dstPort);
            tcpLayer->getTcpHeader()->synFlag = 1;
            tcpLayer->getTcpHeader()->ackFlag = 1;
            ipLayer                           = new pcpp::IPv4Layer();
            ipLayer->setDstIPv4Address(stateMachine->connection->dstAddr);
            layers->push(tcpLayer);
            layers->push(ipLayer);
            stateMachine->connection->owner->sendPacket(layers);

            stateMachine->changeState(new TCPStateSYNReceived(stateMachine));


            break;

        default:
            handled = false;
            break;
    }
    return handled;
}