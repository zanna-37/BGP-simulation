#include "TCPStateFINWait2.h"

#include <stack>

#include "../../logger/Logger.h"
#include "../TCPFlag.h"
#include "TCPStateTimeWait.h"

TCPStateFINWait2::TCPStateFINWait2(TCPStateMachine* stateMachine)
    : TCPState(stateMachine) {
    NAME = "FIN-WAIT-2";
    L_DEBUG("State created: " + NAME);
}

bool TCPStateFINWait2::onEvent(TCPEvent event) {
    std::stack<pcpp::Layer*>* layers   = nullptr;
    pcpp::TcpLayer*           tcpLayer = nullptr;

    bool handled = true;
    switch (event) {
        case TCPEvent::ReceiveFINSendACK:
            // The device receives a FIN from the other device. It acknowledges
            // it and moves to the TIME-WAIT state.
            layers   = new std::stack<pcpp::Layer*>();
            tcpLayer = craftTCPLayer(stateMachine->connection->srcPort,
                                     stateMachine->connection->dstPort,
                                     FIN + ACK);
            layers->push(tcpLayer);
            stateMachine->connection->owner->sendPacket(
                layers, stateMachine->connection->dstAddr.toString());
            stateMachine->timeWaitTimer->start();
            stateMachine->changeState(new TCPStateTimeWait(stateMachine));

            break;

        default:
            handled = false;
            break;
    }
    return handled;
}