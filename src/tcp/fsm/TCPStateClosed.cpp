#include "TCPStateClosed.h"


bool TCPStateClosed::onEvent(TCPEvent event) {
    bool handled = true;

    // FIXME create Random open port
    uint16_t              srcPort  = 12345;
    pcpp::TcpLayer *      tcpLayer = nullptr;
    pcpp::IPv4Layer *     ipLayer  = nullptr;
    stack<pcpp::Layer *> *layers   = nullptr;

    switch (event) {
        case PassiveOpen:
            // A server begins the process of connection setup by doing a
            // passive open on a TCP port. At the same time, it sets up the data
            // structure (transmission control block or TCB) needed to manage
            // the connection. It then transitions to the LISTEN state.

            stateMachine->changeState(new TCPStateListen(stateMachine));

            break;
        case ActiveOpen_SendSYN:
            // A client begins connection setup by sending a SYN message, and
            // also sets up a TCB for this connection. It then transitions to
            // the SYN-SENT state.
            layers = new stack<pcpp::Layer *>();
            tcpLayer =
                new pcpp::TcpLayer(srcPort, stateMachine->connection->dstPort);
            ipLayer                           = new pcpp::IPv4Layer();
            tcpLayer->getTcpHeader()->synFlag = 1;
            ipLayer->setDstIPv4Address(stateMachine->connection->dstAddr);
            layers->push(tcpLayer);
            layers->push(ipLayer);
            stateMachine->connection->owner->sendPacket(layers);
            stateMachine->changeState(new TCPStateSYNSent(stateMachine));
            break;
        default:
            handled = false;
            break;
    }
    return handled;
}