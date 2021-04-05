#include "TCPStateCloseWait.h"

#include <IPLayer.h>
#include <TcpLayer.h>

#include <stack>

#include "../../logger/Logger.h"
#include "TCPStateLastACK.h"

TCPStateCloseWait::TCPStateCloseWait(TCPStateMachine *stateMachine)
    : TCPState(stateMachine) {
    NAME = "CLOSE_WAIT";
    L_DEBUG("State created: " + NAME);
}

bool TCPStateCloseWait::onEvent(TCPEvent event) {
    bool handled = true;


    pcpp::TcpLayer *      tcpLayer = nullptr;
    pcpp::IPv4Layer *     ipLayer  = nullptr;
    stack<pcpp::Layer *> *layers   = nullptr;
    switch (event) {
        case TCPEvent::CloseSendFIN:
            // The application using TCP, having been informed the other process
            // wants to shut down, sends a close request to the TCP layer on the
            // machine upon which it is running. TCP then sends a FIN to the
            // remote device that already asked to terminate the connection.
            // This device now transitions to LAST-ACK.

            layers = new std::stack<pcpp::Layer *>();

            tcpLayer = new pcpp::TcpLayer(stateMachine->connection->srcPort,
                                          stateMachine->connection->dstPort);
            tcpLayer->getTcpHeader()->finFlag = 1;
            ipLayer                           = new pcpp::IPv4Layer();
            ipLayer->setDstIPv4Address(stateMachine->connection->dstAddr);
            layers->push(tcpLayer);
            layers->push(ipLayer);

            stateMachine->connection->owner->sendPacket(layers);

            stateMachine->changeState(new TCPStateLastACK(stateMachine));
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}
