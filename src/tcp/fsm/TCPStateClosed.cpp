#include "TCPStateClosed.h"

#include <stack>

#include "../../logger/Logger.h"
#include "../TCPFlag.h"
#include "TCPStateClosed.h"
#include "TCPStateListen.h"
#include "TCPStateSYNSent.h"

TCPStateClosed::TCPStateClosed(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    NAME = "CLOSED";
    L_DEBUG("State created: " + NAME);
}

bool TCPStateClosed::onEvent(TCPEvent event) {
    bool handled = true;


    std::stack<pcpp::Layer*>* layers   = nullptr;
    pcpp::TcpLayer*           tcpLayer = nullptr;

    switch (event) {
        case TCPEvent::PassiveOpen:
            // A server begins the process of connection setup by doing a
            // passive open on a TCP port. At the same time, it sets up the data
            // structure (transmission control block or TCB) needed to manage
            // the connection. It then transitions to the LISTEN state.

            stateMachine->changeState(new TCPStateListen(stateMachine));

            break;
        case TCPEvent::ActiveOpen_SendSYN:
            // A client begins connection setup by sending a SYN message, and
            // also sets up a TCB for this connection. It then transitions to
            // the SYN-SENT state.

            layers   = new std::stack<pcpp::Layer*>();
            tcpLayer = craftTCPLayer(stateMachine->connection->srcPort,
                                     stateMachine->connection->dstPort,
                                     SYN);
            layers->push(tcpLayer);
            stateMachine->connection->owner->sendPacket(
                layers, stateMachine->connection->dstAddr->toString());
            stateMachine->changeState(new TCPStateSYNSent(stateMachine));
            delete layers;
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