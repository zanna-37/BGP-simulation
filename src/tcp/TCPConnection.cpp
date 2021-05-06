#include "TCPConnection.h"

#include <TcpLayer.h>

#include "../logger/Logger.h"
#include "TCPEvent.h"
#include "TCPFlag.h"
#include "fsm/TCPStateClosed.h"


TCPConnection ::TCPConnection(Device* owner) : owner(owner) {
    this->stateMachine = new TCPStateMachine(this);
    this->stateMachine->changeState(new TCPStateClosed(this->stateMachine));
    this->stateMachine->start();
}

TCPConnection::~TCPConnection() {
    delete stateMachine;

    stopThread();
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

    processFlags(flags, layers);

    delete ipLayer;
    delete tcpLayer;
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

void TCPConnection::listen() {
    enqueueEvent(TCPEvent::PassiveOpen);
    start();
}


bool TCPConnection::isReady() {
    bool result;
    ready_mutex.lock();
    result = ready;
    ready_mutex.unlock();
    return result;
}

bool TCPConnection::isConnected() {
    bool result;
    connected_mutex.lock();
    result = connected;
    connected_mutex.unlock();
    return result;
}

void TCPConnection::setConnected(bool value) {
    connected_mutex.lock();
    connected = value;
    connected_mutex.unlock();
}

void TCPConnection::accept() {
    // connection is ready to be accepted
    ready_mutex.lock();
    ready = true;
    ready_mutex.unlock();

    std::unique_lock<std::mutex> receivingQueue_uniqueLock(
        receivingQueue_mutex);
    receivingQueue_wakeup.notify_one();
    receivingQueue_uniqueLock.unlock();
}

void TCPConnection::connect(const pcpp::IPv4Address& dstAddr,
                            uint16_t                 dstPort) {
    this->dstAddr = dstAddr;
    this->dstPort = dstPort;
    running       = true;

    pcpp::TcpLayer*           tcpLayer = craftTCPLayer(srcPort, 179, SYN);
    std::stack<pcpp::Layer*>* layers   = new std::stack<pcpp::Layer*>();
    layers->push(tcpLayer);
    sendPacket(layers);
    enqueueEvent(TCPEvent::ActiveOpen_SendSYN);
}


pcpp::TcpLayer* TCPConnection::craftTCPLayer(uint16_t srcPort,
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

void TCPConnection::processFlags(uint8_t                   flags,
                                 std::stack<pcpp::Layer*>* applicationLayers) {
    std::stack<pcpp::Layer*>* layers   = new std::stack<pcpp::Layer*>();
    pcpp::TcpLayer*           tcpLayer = nullptr;
    if (flags == SYN) {
        tcpLayer = craftTCPLayer(srcPort, dstPort, SYN + ACK);
        layers->push(tcpLayer);
        sendPacket(layers);
        enqueueEvent(TCPEvent::ReceiveClientSYN_SendSYNACK);
    } else if (flags == SYN + ACK) {
        tcpLayer = craftTCPLayer(srcPort, dstPort, ACK);
        layers->push(tcpLayer);
        sendPacket(layers);
        enqueueEvent(TCPEvent::ReceiveSYNACKSendACK);
    } else if (flags == ACK) {
        enqueueEvent(TCPEvent::ReceiveACK);
    } else if (flags == FIN) {
        tcpLayer = craftTCPLayer(srcPort, dstPort, FIN + ACK);
        layers->push(tcpLayer);
        sendPacket(layers);
        enqueueEvent(TCPEvent::ReceiveFINSendACK);
        // TODO, change it
        closeConnection();
    } else if (flags == FIN + ACK) {
        enqueueEvent(TCPEvent::ReceiveACKforFIN);
    } else if (flags == RST) {
        enqueueEvent(TCPEvent::ReceiveRST);
        closeConnection();
    } else if (flags == PSH + ACK && isConnected()) {
        tcpLayer = craftTCPLayer(srcPort, dstPort, ACK);
        layers->push(tcpLayer);
        sendPacket(layers);
        // Application layer will handle the message
        enqueueApplicationLayers(applicationLayers);
        // socket.recv(applicationLayers)
    } else {
        L_ERROR(owner->ID, "TCP flag combination not handled");
    }
}

void TCPConnection::start() {
    running = true;


    receivingThread = new std::thread([&]() {
        while (running) {
            std::unique_lock<std::mutex> receivingQueue_uniqueLock(
                receivingQueue_mutex);
            while ((receivingQueue.empty() || !isReady()) && running) {
                receivingQueue_wakeup.wait(receivingQueue_uniqueLock);

                if ((receivingQueue.empty() || !isReady()) && running) {
                    L_DEBUG(name, "Spurious wakeup");
                }
            }

            if (running) {
                if (isReady()) {
                    std::stack<pcpp::Layer*>* layers = receivingQueue.front();
                    receivingQueue.pop();
                    processMessage(layers);
                } else {
                    owner->notifyListeningSocket(this);
                }
            } else {
                L_DEBUG(name, "Shutting Down receiving queue");
            }
        }
    });

    sendingThread = new std::thread([&]() {
        while (running) {
            std::unique_lock<std::mutex> sendingQueue_uniqueLock(
                sendingQueue_mutex);
            while (sendingQueue.empty() && running) {
                sendingQueue_wakeup.wait(sendingQueue_uniqueLock);

                if (sendingQueue.empty() && running) {
                    L_DEBUG(name, "Spurious wakeup");
                }
            }

            if (running) {
                std::stack<pcpp::Layer*>* layers = sendingQueue.front();
                sendingQueue.pop();
                owner->sendPacket(layers, dstAddr);
                delete layers;
            } else {
                L_DEBUG(name, "Shutting Down sending queue");
            }
        }
    });
}

void TCPConnection::sendPacket(std::stack<pcpp::Layer*>* layers) {
    std::unique_lock<std::mutex> sendingQueue_uniqueLock(sendingQueue_mutex);
    sendingQueue.push(layers);
    sendingQueue_wakeup.notify_one();
    sendingQueue_uniqueLock.unlock();
}

void TCPConnection::receivePacket(std::stack<pcpp::Layer*>* layers) {
    std::unique_lock<std::mutex> receivingQueue_uniqueLock(
        receivingQueue_mutex);
    receivingQueue.push(layers);
    receivingQueue_wakeup.notify_one();
    receivingQueue_uniqueLock.unlock();
}


void TCPConnection::closeConnection() {
    Socket* s = owner->getAssociatedConnectedSocketOrNull(this);

    if (s != nullptr) {
        s->applicationConnection->closeConnection();
    } else {
        L_FATAL(name, "No connected socket associated to the connection");
    }
}

std::stack<pcpp::Layer*>* TCPConnection::waitForApplicationData() {
    std::unique_lock<std::mutex> appReceivingQueue_uniqueLock(
        appReceivingQueue_mutex);

    while (appReceivingQueue.empty() && running) {
        appReceivingQueue_wakeup.wait(appReceivingQueue_uniqueLock);

        if (appReceivingQueue.empty() && running) {
            L_DEBUG(name, "Spurious wakeup");
        }
    }

    std::stack<pcpp::Layer*>* layers = appReceivingQueue.front();
    appReceivingQueue.pop();

    return layers;
}

void TCPConnection::enqueueApplicationLayers(std::stack<pcpp::Layer*>* layers) {
    std::unique_lock<std::mutex> appReceivingQueue_uniqueLock(
        appReceivingQueue_mutex);

    appReceivingQueue.push(layers);
    appReceivingQueue_wakeup.notify_one();
    appReceivingQueue_uniqueLock.unlock();
}

void TCPConnection::sendApplicationData(std::stack<pcpp::Layer*>* layers) {
    pcpp::TcpLayer* tcpLayer = craftTCPLayer(srcPort, dstPort, PSH + ACK);
    layers->push(tcpLayer);
    sendPacket(layers);
}