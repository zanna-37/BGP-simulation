#include "TCPConnection.h"

#include <TcpLayer.h>

#include "../logger/Logger.h"
#include "TCPEvent.h"
#include "TCPFlag.h"
#include "fsm/TCPStateClosed.h"


TCPConnection::TCPConnection(Device* owner) : owner(owner) {
    this->stateMachine = new TCPStateMachine(this);
    this->stateMachine->changeState(new TCPStateClosed(this->stateMachine));
    this->stateMachine->start();
}

TCPConnection::~TCPConnection() { delete stateMachine; }

void TCPConnection::enqueueEvent(TCPEvent event) {
    stateMachine->enqueueEvent(event);
}

TCPState* TCPConnection::getCurrentState() {
    return stateMachine->getCurrentState();
}


void TCPConnection::processMessage(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> receivedLayers) {
    auto layer = std::move(receivedLayers->top());
    receivedLayers->pop();

    auto* receivedTcpLayer_weak = dynamic_cast<pcpp::TcpLayer*>(layer.get());

    uint8_t receivedFlags =
        parseTCPFlags(receivedTcpLayer_weak->getTcpHeader());

    // Preparing the response
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layersToSend;

    if (receivedFlags == SYN) {
        layersToSend->push(craftTCPLayer(srcPort, dstPort, SYN + ACK));
        sendPacket(std::move(layersToSend));
        enqueueEvent(TCPEvent::ReceiveClientSYN_SendSYNACK);

    } else if (receivedFlags == SYN + ACK) {
        layersToSend->push(craftTCPLayer(srcPort, dstPort, ACK));
        sendPacket(std::move(layersToSend));
        enqueueEvent(TCPEvent::ReceiveSYNACKSendACK);

    } else if (receivedFlags == ACK) {
        enqueueEvent(TCPEvent::ReceiveACK);

    } else if (receivedFlags == FIN) {
        layersToSend->push(craftTCPLayer(srcPort, dstPort, FIN + ACK));
        sendPacket(std::move(layersToSend));
        enqueueEvent(TCPEvent::ReceiveFINSendACK);
        // TODO, change it
        closeConnection();

    } else if (receivedFlags == FIN + ACK) {
        enqueueEvent(TCPEvent::ReceiveACKforFIN);

    } else if (receivedFlags == RST) {
        enqueueEvent(TCPEvent::ReceiveRST);
        closeConnection();

    } else if (receivedFlags == PSH + ACK && isConnected()) {
        layersToSend->push(craftTCPLayer(srcPort, dstPort, ACK));
        sendPacket(std::move(layersToSend));

        // Application layer will handle the message
        enqueueApplicationLayers(std::move(receivedLayers));
        // socket.recv(receivedLayers)

    } else {
        L_ERROR(owner->ID, "TCP flag combination not handled");
    }
    // TODO handle application layers even if they do not have the PUSH flag
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

    std::unique_ptr<pcpp::Layer> tcpLayer = craftTCPLayer(srcPort, 179, SYN);
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers;
    layers->push(std::move(tcpLayer));
    sendPacket(std::move(layers));
    enqueueEvent(TCPEvent::ActiveOpen_SendSYN);
}


std::unique_ptr<pcpp::TcpLayer> TCPConnection::craftTCPLayer(uint16_t srcPort,
                                                             uint16_t dstPort,
                                                             int      flags) {
    std::unique_ptr<pcpp::TcpLayer> tcpLayer =
        std::make_unique<pcpp::TcpLayer>(srcPort, dstPort);

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

    return std::move(tcpLayer);
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
                    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                        layers = std::move(receivingQueue.front());
                    receivingQueue.pop();
                    processMessage(std::move(layers));
                } else {
                    // connection is ready to be accepted
                    ready_mutex.lock();
                    ready = true;
                    ready_mutex.unlock();
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
                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers = std::move(sendingQueue.front());
                sendingQueue.pop();
                owner->sendPacket(std::move(layers), dstAddr);
            } else {
                L_DEBUG(name, "Shutting Down sending queue");
            }
        }
    });
}

void TCPConnection::sendPacket(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    std::unique_lock<std::mutex> sendingQueue_uniqueLock(sendingQueue_mutex);
    sendingQueue.push(std::move(layers));
    sendingQueue_wakeup.notify_one();
    sendingQueue_uniqueLock.unlock();
}

void TCPConnection::receivePacket(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    std::unique_lock<std::mutex> receivingQueue_uniqueLock(
        receivingQueue_mutex);
    receivingQueue.push(std::move(layers));
    receivingQueue_wakeup.notify_one();
    receivingQueue_uniqueLock.unlock();
}


void TCPConnection::closeConnection() {
    // TODO handle the notification to higher level that the TCP connection has
    // been closed

    // Socket* s = owner->getAssociatedConnectedSocketOrNull(this);

    // if (s != nullptr) {
    //     s->applicationConnection->closeConnection();
    // } else {
    //     L_FATAL(name, "No connected socket associated to the connection");
    // }
}

std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
TCPConnection::waitForApplicationData() {
    std::unique_lock<std::mutex> appReceivingQueue_uniqueLock(
        appReceivingQueue_mutex);

    while (appReceivingQueue.empty() && running) {
        appReceivingQueue_wakeup.wait(appReceivingQueue_uniqueLock);

        if (appReceivingQueue.empty() && running) {
            L_DEBUG(name, "Spurious wakeup");
        }
    }

    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers =
        std::move(appReceivingQueue.front());
    appReceivingQueue.pop();

    return std::move(layers);
}

void TCPConnection::enqueueApplicationLayers(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    std::unique_lock<std::mutex> appReceivingQueue_uniqueLock(
        appReceivingQueue_mutex);

    appReceivingQueue.push(std::move(layers));
    appReceivingQueue_wakeup.notify_one();
    appReceivingQueue_uniqueLock.unlock();
}

void TCPConnection::sendApplicationData(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    std::unique_ptr<pcpp::Layer> tcpLayer =
        craftTCPLayer(srcPort, dstPort, PSH + ACK);
    layers->push(std::move(tcpLayer));
    sendPacket(std::move(layers));
}