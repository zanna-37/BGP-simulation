#include "TCPConnection.h"

#include "TCPFlag.h"
#include "fsm/TCPStateClosed.h"


TCPConnection ::TCPConnection(Device* owner) : owner(owner) {
    this->stateMachine = new TCPStateMachine(this);
    this->stateMachine->changeState(new TCPStateClosed(this->stateMachine));
    this->stateMachine->start();
}

TCPConnection::~TCPConnection() {
    delete stateMachine;
    delete srcAddr;
    delete dstAddr;
}

void TCPConnection::enqueueEvent(TCPEvent event) {
    stateMachine->enqueueEvent(event);
}

TCPState* TCPConnection::getCurrentState() {
    return stateMachine->getCurrentState();
}


void TCPConnection::processMessage(std::stack<pcpp::Layer*>* layers) {
    pcpp::IPv4Layer* ipLayer = dynamic_cast<pcpp::IPv4Layer*>(layers->top());
    layers->pop();
    pcpp::TcpLayer* tcpLayer = dynamic_cast<pcpp::TcpLayer*>(layers->top());
    layers->pop();

    pcpp::tcphdr* tcpHeader = tcpLayer->getTcpHeader();
    uint8_t       flags     = parseTCPFlags(tcpHeader);
    if (flags == SYN) {
        srcAddr =
            new pcpp::IPv4Address(ipLayer->getDstIPv4Address().toString());
        srcPort = tcpLayer->getDstPort();
        dstAddr =
            new pcpp::IPv4Address(ipLayer->getSrcIPv4Address().toString());
        dstPort = tcpLayer->getSrcPort();

        owner->addTCPConnection(this);
        enqueueEvent(TCPEvent::ReceiveClientSYN_SendSYNACK);

        // create new listening connection (allocate TCB)
        owner->listenConnection          = new TCPConnection(owner);
        owner->listenConnection->srcPort = this->srcPort;
    } else if (flags == SYN + ACK) {
        enqueueEvent(TCPEvent::ReceiveSYNACKSendACK);
    } else if (flags == ACK) {
        enqueueEvent(TCPEvent::ReceiveACK);
    } else if (flags == FIN) {
        enqueueEvent(TCPEvent::ReceiveFINSendACK);
    } else if (flags == FIN + ACK) {
        enqueueEvent(TCPEvent::ReceiveACKforFIN);
    } else if (flags == RST) {
        enqueueEvent(TCPEvent::ReceiveRST);
    } else {
        L_ERROR("TCP flag combination not handled");
    }  // TODO handle RST flag


    delete ipLayer;
    delete tcpLayer;

    // FIXME
    delete layers;
}


uint8_t TCPConnection::parseTCPFlags(pcpp::tcphdr* tcpHeader) {
    uint8_t result = 0;

    if (tcpHeader->urgFlag == 1) {
        result += 32;
    }
    if (tcpHeader->ackFlag == 1) {
        result += 16;
    }
    if (tcpHeader->pshFlag == 1) {
        result += 8;
    }
    if (tcpHeader->rstFlag == 1) {
        result += 4;
    }
    if (tcpHeader->synFlag == 1) {
        result += 2;
    }
    if (tcpHeader->finFlag == 1) {
        result += 1;
    }

    return result;
}