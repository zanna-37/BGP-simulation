#include "TCPStateFINWait1.h"

#include <stack>

#include "../../logger/Logger.h"
#include "../TCPFlag.h"
#include "TCPStateClosed.h"
#include "TCPStateClosing.h"
#include "TCPStateFINWait2.h"

TCPStateFINWait1::TCPStateFINWait1(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    NAME = "FIN-WAIT-1";
    L_DEBUG("State created: " + NAME);
}
bool TCPStateFINWait1::onEvent(TCPEvent event) {
    std::stack<pcpp::Layer*>* layers   = nullptr;
    pcpp::TcpLayer*           tcpLayer = nullptr;

    bool handled = true;
    switch (event) {
        case TCPEvent::ReceiveACKforFIN:
            // The device receives an acknowledgment for its close request. It
            // transitions to the FIN-WAIT-2 state.
            stateMachine->changeState(new TCPStateFINWait2(stateMachine));

            break;
        case TCPEvent::ReceiveFINSendACK:
            // The device does not receive an ACK for its own FIN, but receives
            // a FIN from the other device. It acknowledges it, and moves to the
            // CLOSING state.
            layers   = new std::stack<pcpp::Layer*>();
            tcpLayer = craftTCPLayer(stateMachine->connection->srcPort,
                                     stateMachine->connection->dstPort,
                                     FIN + ACK);
            layers->push(tcpLayer);
            stateMachine->connection->owner->sendPacket(
                layers, stateMachine->connection->dstAddr->toString());
            delete layers;
            stateMachine->changeState(new TCPStateClosing(stateMachine));

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