#include "TCPConnection.h"

#include <chrono>
#include <thread>
#include <utility>

#include "../entities/Device.h"
#include "../logger/Logger.h"
#include "Layer.h"
#include "TCPEvent.h"
#include "TCPFlag.h"
#include "TcpLayer.h"
#include "fsm/TCPState.h"
#include "fsm/TCPStateClosed.h"
#include "fsm/TCPStateEstablished.h"
#include "fsm/TCPStateListen.h"
#include "fsm/TCPStateMachine.h"


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
                  "closed state. Currently in " +
                      stateMachine->getCurrentState()->name +
                      ".\nCall close() before deleting the connection");
    }

    // wake up everybody just in case. It shouldn't be needed but "better safe
    // than sorry" since a missing .notify_all() freezes the computation.
    appReceivingQueue_wakeup.notify_all();
    pendingConnections_wakeup.notify_all();
    sendingQueue_wakeup.notify_all();
    tryingToEstablish_wakeup.notify_all();

    delete stateMachine;
}

void TCPConnection::abort() {
    stateMachine->enqueueEvent(TCPEvent::Abort);
    // TODO wait for the abort call to be completed
    L_DEBUG(stateMachine->connection->owner->ID,
            "TODO wait for the abort call to be completed");
    std::this_thread::sleep_for(3000ms);  // TODO remove
}

void TCPConnection::close() {
    stateMachine->enqueueEvent(TCPEvent::Close);
    // TODO wait for the close call to be completed
    L_DEBUG(stateMachine->connection->owner->ID,
            "TODO wait for the close call to be completed");
    std::this_thread::sleep_for(3000ms);  // TODO remove
}

void TCPConnection::segmentArrives(
    std::pair<pcpp::IPv4Address,
              std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>>
        receivedLayers) {
    receivingQueue_mutex.lock();
    receivingQueue.push(std::move(receivedLayers));
    receivingQueue_mutex.unlock();

    stateMachine->enqueueEvent(TCPEvent::SegmentArrives);
}

std::pair<pcpp::IPv4Address,
          std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>>
TCPConnection::getNextSegment() {
    std::pair<pcpp::IPv4Address,
              std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>>
        result;

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

    // TODO wait for the listen call to be completed and return result
    L_DEBUG(stateMachine->connection->owner->ID,
            "TODO wait for the listen call to be completed and return result");
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

    // TODO wait until the connection is established and return result
    L_DEBUG(stateMachine->connection->owner->ID,
            "TODO wait until the connection is established and return result");
    std::this_thread::sleep_for(200ms);  // TODO remove

    return 0;
}

std::unique_ptr<pcpp::TcpLayer> TCPConnection::craftTCPLayer(uint16_t srcPort,
                                                             uint16_t dstPort,
                                                             int      flags) {
    std::unique_ptr<pcpp::TcpLayer> tcpLayer =
        std::make_unique<pcpp::TcpLayer>(srcPort, dstPort);

    switch (flags) {
        case TCPFlag::SYN:
            tcpLayer->getTcpHeader()->synFlag = 1;
            break;
        case TCPFlag::SYN + TCPFlag::ACK:
            tcpLayer->getTcpHeader()->synFlag = 1;
            tcpLayer->getTcpHeader()->ackFlag = 1;
            break;
        case TCPFlag::ACK:
            tcpLayer->getTcpHeader()->ackFlag = 1;
            break;
        case TCPFlag::FIN:
            tcpLayer->getTcpHeader()->finFlag = 1;
            break;
        case TCPFlag::FIN + TCPFlag::ACK:
            tcpLayer->getTcpHeader()->finFlag = 1;
            tcpLayer->getTcpHeader()->ackFlag = 1;
            break;
        case TCPFlag::RST:
            tcpLayer->getTcpHeader()->rstFlag = 1;
            break;
        case TCPFlag::PSH + TCPFlag::ACK:
            tcpLayer->getTcpHeader()->pshFlag = 1;
            tcpLayer->getTcpHeader()->ackFlag = 1;
            break;
        default:
            break;
    }

    return std::move(tcpLayer);
}

void TCPConnection::enqueuePacketToPeerOutbox(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    enqueuePacketToOutbox(std::move(layers), dstAddr);
}

void TCPConnection::enqueuePacketToOutbox(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers,
    pcpp::IPv4Address&                                        dstAddr) {
    owner->sendPacket(std::move(layers), dstAddr);
}

std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
TCPConnection::waitForApplicationData() {
    std::unique_lock<std::mutex> appReceivingQueue_uniqueLock(
        appReceivingQueue_mutex);

    assert(dstPort != 0);  // Waiting data from a listening-only connection? Use
                           // socket.accept();
    while (appReceivingQueue.empty() && running) {
        appReceivingQueue_wakeup.wait(appReceivingQueue_uniqueLock);

        if (appReceivingQueue.empty() && running) {
            L_DEBUG(name, "Spurious wakeup");
        }
    }

    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers;
    if (!appReceivingQueue.empty()) {
        layers = std::move(appReceivingQueue.front());
        appReceivingQueue.pop();
    }
    return std::move(layers);
}

void TCPConnection::enqueuePacketToInbox(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    appReceivingQueue_mutex.lock();
    appReceivingQueue.push(std::move(layers));
    appReceivingQueue_mutex.unlock();

    appReceivingQueue_wakeup.notify_one();
}

std::shared_ptr<TCPConnection>
TCPConnection::createConnectedConnectionFromListening(
    const pcpp::IPv4Address& dstAddr, const uint16_t dstPort) {
    // TODO use this in the listening state if we receive a SYN
    // TODO the caller (so not this function) need to push the new connection
    // into a pendingConnection queue and notify the accept() method.

    std::shared_ptr<TCPConnection> newTcpConnection =
        std::make_shared<TCPConnection>(owner);
    newTcpConnection->srcAddr = srcAddr;
    newTcpConnection->srcPort = srcPort;
    newTcpConnection->dstAddr = dstAddr;
    newTcpConnection->dstPort = dstPort;

    // Make the connection as it would have been in a listening state.
    newTcpConnection->running = true;
    newTcpConnection->stateMachine->changeState(
        new TCPStateListen(newTcpConnection->stateMachine));

    return newTcpConnection;
}

void TCPConnection::send(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    appSendingQueue_mutex.lock();
    appSendingQueue.push(std::move(layers));
    appSendingQueue_mutex.unlock();

    stateMachine->enqueueEvent(TCPEvent::Send);
}

void TCPConnection::sendSynToPeer() {
    L_DEBUG(stateMachine->connection->owner->ID,
            "Sending SYN to " + stateMachine->connection->dstAddr.toString() +
                ":" + std::to_string(stateMachine->connection->dstPort));

    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers =
        std::make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();

    std::unique_ptr<pcpp::Layer> tcpLayer =
        craftTCPLayer(srcPort, dstPort, TCPFlag::SYN);

    layers->push(std::move(tcpLayer));
    enqueuePacketToOutbox(std::move(layers), dstAddr);
}

void TCPConnection::sendFinToPeer() {
    L_DEBUG(stateMachine->connection->owner->ID,
            "Sending FIN to " + stateMachine->connection->dstAddr.toString() +
                ":" + std::to_string(stateMachine->connection->dstPort));

    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers =
        std::make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();

    std::unique_ptr<pcpp::Layer> tcpLayer =
        craftTCPLayer(srcPort, dstPort, TCPFlag::FIN);

    layers->push(std::move(tcpLayer));
    enqueuePacketToOutbox(std::move(layers), dstAddr);
}

void TCPConnection::sendAckToPeer() {
    L_DEBUG(stateMachine->connection->owner->ID,
            "Sending ACK to " + stateMachine->connection->dstAddr.toString() +
                ":" + std::to_string(stateMachine->connection->dstPort));

    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers =
        std::make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();

    std::unique_ptr<pcpp::Layer> tcpLayer =
        craftTCPLayer(srcPort, dstPort, TCPFlag::ACK);

    layers->push(std::move(tcpLayer));
    enqueuePacketToOutbox(std::move(layers), dstAddr);
}

void TCPConnection::sendSynAckToPeer() {
    L_DEBUG(stateMachine->connection->owner->ID,
            "Sending SYN+ACK to " +
                stateMachine->connection->dstAddr.toString() + ":" +
                std::to_string(stateMachine->connection->dstPort));

    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers =
        std::make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();

    std::unique_ptr<pcpp::Layer> tcpLayer =
        craftTCPLayer(srcPort, dstPort, TCPFlag::SYN + TCPFlag::ACK);

    layers->push(std::move(tcpLayer));
    enqueuePacketToOutbox(std::move(layers), dstAddr);
}
void TCPConnection::sendRstToPeer() { sendRstTo(dstAddr, dstPort); }

void TCPConnection::sendRstTo(pcpp::IPv4Address dstAddr, uint16_t dstPort) {
    L_DEBUG(
        stateMachine->connection->owner->ID,
        "Sending RST to " + dstAddr.toString() + ":" + std::to_string(dstPort));

    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers =
        std::make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();

    std::unique_ptr<pcpp::Layer> tcpLayer =
        craftTCPLayer(srcPort, dstPort, TCPFlag::RST);

    layers->push(std::move(tcpLayer));
    enqueuePacketToOutbox(std::move(layers), dstAddr);
}
