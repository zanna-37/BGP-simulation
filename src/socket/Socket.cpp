#include "Socket.h"

#include <cassert>
#include <utility>

#include "../entities/Device.h"
#include "../entities/NetworkCard.h"
#include "../logger/Logger.h"
#include "../tcp/TCPConnection.h"
#include "Layer.h"

Socket::Socket(Device* device) : device(device) {
    tcpConnection = std::make_shared<TCPConnection>(device);
}

Socket::Socket(Device* device, std::shared_ptr<TCPConnection> tcpConnection)
    : device(device), tcpConnection(std::move(tcpConnection)) {}

Socket::~Socket() {
    if (running) {
        close();
        L_ERROR(device->ID,
                "Socket has not been stopped before deletion.\nCall close() "
                "before deleting the Socket.");
    }
}


void Socket::close() {
    running = false;
    tcpConnection->close();
}

int Socket::listen() { return tcpConnection->listen(); }

int Socket::bind(const pcpp::IPv4Address& srcAddr, const uint16_t& srcPort) {
    tcpConnection->srcAddr = srcAddr;
    tcpConnection->srcPort = srcPort;

    return device->bind(tcpConnection);
}

Socket* Socket::accept() {
    assert(tcpConnection);
    std::shared_ptr<TCPConnection> newTcpConnection = tcpConnection->accept();
    if (newTcpConnection) {
        auto* connectedSocket = new Socket(device, newTcpConnection);
        return connectedSocket;
    } else {
        return nullptr;
    }
}

int Socket::connect(const pcpp::IPv4Address& dstAddr, uint16_t dstPort) {
    NetworkCard* nextHopNetworkCard =
        device->getNextHopNetworkCardOrNull(dstAddr);
    if (nextHopNetworkCard == nullptr) {
        L_ERROR(device->ID, dstAddr.toString() + ": Destination unreachable");
        return 1;
    } else {
        tcpConnection->srcAddr = nextHopNetworkCard->IP;
        // tcpConnection->srcPort is already 0
        tcpConnection->dstAddr = dstAddr;
        tcpConnection->dstPort = dstPort;
        device->bind(tcpConnection);

        std::unique_lock<std::mutex> tcpConnection_uniqueLock(
            tcpConnection_mutex);

        return tcpConnection->connect();
    }
}

void Socket::dataArrived() {
    // TODO DELME
    std::unique_lock<std::mutex> tcpConnection_uniqueLock(tcpConnection_mutex);
    tcpConnection_wakeup.notify_one();
    tcpConnection_uniqueLock.unlock();
}

std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> Socket::recv() {
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers =
        tcpConnection->waitForApplicationData();
    return layers;
}


void Socket::send(std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                      applicationLayers) {
    tcpConnection->send(std::move(applicationLayers));
}
