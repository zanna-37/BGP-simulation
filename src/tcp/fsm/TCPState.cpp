#include "TCPState.h"

#include "../TCPFlag.h"
pcpp::TcpLayer* TCPState::craftTCPLayer(uint16_t srcPort,
                                        uint16_t dstPort,
                                        int      flags) {
    pcpp::TcpLayer* tcpLayer = new pcpp::TcpLayer(srcPort, dstPort);


    switch (flags) {
        case SYN:
            tcpLayer->getTcpHeader()->synFlag = 1;
            break;
        case SYN + ACK:
            tcpLayer->getTcpHeader()->synFlag = 1;
            tcpLayer->getTcpHeader()->ackFlag = 1;
            break;
        case ACK:
            tcpLayer->getTcpHeader()->ackFlag = 1;
            break;
        case FIN:
            tcpLayer->getTcpHeader()->finFlag = 1;
            break;
        case FIN + ACK:
            tcpLayer->getTcpHeader()->finFlag = 1;
            tcpLayer->getTcpHeader()->ackFlag = 1;
            break;
        case RST:
            tcpLayer->getTcpHeader()->rstFlag = 1;
            break;
        case PSH + ACK:
            tcpLayer->getTcpHeader()->pshFlag = 1;
            tcpLayer->getTcpHeader()->ackFlag = 1;
            break;
        default:
            break;
    }

    return tcpLayer;
}