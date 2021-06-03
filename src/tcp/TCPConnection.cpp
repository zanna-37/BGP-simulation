#include "TCPConnection.h"

#include <TcpLayer.h>

#include "../logger/Logger.h"
#include "TCPEvent.h"
#include "TCPFlag.h"
#include "fsm/TCPStateClosed.h"
#include "fsm/TCPStateEnstablished.h"
#include "fsm/TCPStateListen.h"


TCPConnection::TCPConnection(Device* owner) : owner(owner) {
    this->stateMachine = new TCPStateMachine(this);
    // start() cannot be called in the constructor because we need a fully
    // constructed object
    this->stateMachine->start();
}

TCPConnection::~TCPConnection() {
    if (!dynamic_cast<TCPStateClosed*>(stateMachine->getCurrentState())) {
        L_WARNING("TCP",
                  "TCPConnection has been deallocated without being in the "
                  "closed state.\nCall close() before deleting the connection");
    }
    delete stateMachine;
}

void TCPConnection::close() {
    stateMachine->enqueueEvent(TCPEvent::Close);
    // TODO wait for event to be completed
    std::this_thread::sleep_for(200ms);  // TODO remove
}

void TCPConnection::segmentArrives(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> receivedLayers) {
    receivingQueue_mutex.lock();
    receivingQueue.push(std::move(receivedLayers));
    receivingQueue_mutex.unlock();

    stateMachine->enqueueEvent(TCPEvent::SegmentArrives);
}

std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
TCPConnection::getNextSegment() {
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> result;

    receivingQueue_mutex.lock();
    result = std::move(receivingQueue.front());
    receivingQueue.pop();
    receivingQueue_mutex.unlock();

    return result;
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

int TCPConnection::listen() {
    stateMachine->enqueueEvent(TCPEvent::OpenPassive);

    // TODO wait for event result (success/fail and return the result)
    std::this_thread::sleep_for(200ms);  // TODO remove
    return 0;
}

std::shared_ptr<TCPConnection> TCPConnection::accept() {
    std::unique_lock<std::mutex> pendingConnections_uniqueLock(
        pendingConnections_mutex);

    while (pendingConnections.empty() && running) {
        pendingConnections_wakeup.wait(pendingConnections_uniqueLock);
    }

    std::shared_ptr<TCPConnection> nextPendingConnection;
    if (running) {
        nextPendingConnection = pendingConnections.front();
        pendingConnections.pop();
    }
    return nextPendingConnection;
}

int TCPConnection::connect() {
    stateMachine->enqueueEvent(TCPEvent::OpenActive);

    // Wait until the connection is established
    std::unique_lock<std::mutex> established_uniqueLock(established_mutex);
    while (
        !dynamic_cast<TCPStateEnstablished*>(stateMachine->getCurrentState()) &&
        running) {
        established_wakeup.wait(established_uniqueLock);
    }

    if (running) {
        return 0;
    } else {
        return 1;
    }
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

void TCPConnection::enqueuePacketToOutbox(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    owner->sendPacket(std::move(layers), dstAddr);
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

void TCPConnection::signalApplicationLayersReady(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    std::unique_lock<std::mutex> appReceivingQueue_uniqueLock(
        appReceivingQueue_mutex);

    appReceivingQueue.push(std::move(layers));
    appReceivingQueue_wakeup.notify_one();
    appReceivingQueue_uniqueLock.unlock();
}

shared_ptr<TCPConnection> TCPConnection::createConnectedConnectionFromListening(
    const pcpp::IPv4Address& dstAddr, const uint16_t dstPort) {
    // TODO use this in the listening state if we receive a SYN
    // TODO the caller (so not this function) need to push the new connection
    // into a pendingConnection queue and notify the accept() method.

    shared_ptr<TCPConnection> newTcpConnection =
        make_shared<TCPConnection>(owner);
    newTcpConnection->srcAddr = srcAddr;
    newTcpConnection->srcPort = srcPort;
    newTcpConnection->dstAddr = dstAddr;
    newTcpConnection->dstPort = dstPort;

    // Make the connection as it would have been in a listening state.
    stateMachine->enqueueEvent(TCPEvent::OpenPassive);

    return newTcpConnection;
}

void TCPConnection::send(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    appSendingQueue_mutex.lock();
    appSendingQueue.push(std::move(layers));
    appSendingQueue_mutex.unlock();

    stateMachine->enqueueEvent(TCPEvent::Send);
}

void TCPConnection::sendSyn() {
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers =
        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();

    std::unique_ptr<pcpp::Layer> tcpLayer =
        craftTCPLayer(srcPort, dstPort, SYN);
    layers->push(std::move(tcpLayer));
    enqueuePacketToOutbox(std::move(layers));

    L_DEBUG(stateMachine->connection->owner->ID,
            "SYN sent to " + stateMachine->connection->dstAddr.toString() +
                ":" + std::to_string(stateMachine->connection->dstPort));
}

void TCPConnection::sendFin() {
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers =
        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();

    std::unique_ptr<pcpp::Layer> tcpLayer =
        craftTCPLayer(srcPort, dstPort, FIN);
    layers->push(std::move(tcpLayer));
    enqueuePacketToOutbox(std::move(layers));

    L_DEBUG(stateMachine->connection->owner->ID,
            "FIN sent to " + stateMachine->connection->dstAddr.toString() +
                ":" + std::to_string(stateMachine->connection->dstPort));
}

void TCPConnection::sendRst() {
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers =
        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();

    std::unique_ptr<pcpp::Layer> tcpLayer =
        craftTCPLayer(srcPort, dstPort, RST);
    layers->push(std::move(tcpLayer));
    enqueuePacketToOutbox(std::move(layers));

    L_DEBUG(stateMachine->connection->owner->ID,
            "RST sent to " + stateMachine->connection->dstAddr.toString() +
                ":" + std::to_string(stateMachine->connection->dstPort));
}
