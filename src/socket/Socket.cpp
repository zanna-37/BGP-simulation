#include "Socket.h"

#include "../logger/Logger.h"

Socket::Socket(int type, int domain) : type(type), domain(domain) {}

Socket::~Socket() { running = false; }
int Socket::listen() {
    TCPConnection* tcpConnection =
        device->getNewTCPConnection(srcAddr, srcPort);

    std::unique_lock<std::mutex> tcpConnection_uniqueLock(tcpConnection_mutex);


    tcpConnection->listen();
    // waiting for a pending SYN packet
    while (!tcpConnection->isReady()) {
        tcpConnection_wakeup.wait(tcpConnection_uniqueLock);

        if (!tcpConnection->isReady()) {
            L_DEBUG(name, "Spurious wakeup");
        }
    }

    if (running) {
        // TODO handle possible failed listening (?), which return -1
        return 0;

    } else {
        L_DEBUG(name, "Shutting Down");
        return -1;
    }

    // unique_lock is unlocked automatically
}

int Socket::bind(const pcpp::IPv4Address& srcAddr, uint16_t srcPort) {
    this->srcAddr = srcAddr;
    this->srcPort = srcPort;
    return 0;
}

Socket* Socket::accept() {
    TCPConnection* connection = device->getAssociatedTCPconnectionOrNull(this);

    if (connection != nullptr) {
        std::unique_lock<std::mutex> tcpConnection_uniqueLock(
            tcpConnection_mutex);
        connection->accept();
        while (!connection->isConnected()) {
            tcpConnection_wakeup.wait(tcpConnection_uniqueLock);

            if (!connection->isConnected()) {
                L_DEBUG(name, "Spurious wakeup");
            }
        }

        Socket* connectedSocket =
            new Socket(Socket::Domain::AF_INET, Socket::Type::SOCK_STREAM);

        connectedSocket->srcAddr = connection->srcAddr;
        connectedSocket->dstAddr = connection->dstAddr;
        connectedSocket->srcPort = connection->srcPort;
        connectedSocket->dstPort = connection->dstPort;
        device->connectedSockets.push_back(connectedSocket);

        return connectedSocket;
    } else {
        L_FATAL(name, "No TCPConnection associated with the socket");
    }

    return nullptr;
}

int Socket::connect(const pcpp::IPv4Address& dstAddr, uint16_t dstPort) {
    // FIXME get random srcPort
    uint16_t       srcPort = 12345;
    TCPConnection* connection =
        device->getNewTCPConnection(pcpp::IPv4Address::Zero, srcPort);
    std::unique_lock<std::mutex> tcpConnection_uniqueLock(tcpConnection_mutex);
    connection->connect(dstAddr, dstPort);
    while (!connection->isConnected()) {
        tcpConnection_wakeup.wait(tcpConnection_uniqueLock);

        if (!connection->isConnected()) {
            L_DEBUG(name, "Spurious wakeup");
        }
    }
    this->srcAddr = connection->srcAddr;
    this->srcPort = connection->srcPort;
    this->dstAddr = connection->dstAddr;
    this->dstPort = connection->dstPort;
    return 0;
}

void Socket::dataArrived() {
    std::unique_lock<std::mutex> tcpConnection_uniqueLock(tcpConnection_mutex);
    tcpConnection_wakeup.notify_one();
    tcpConnection_uniqueLock.unlock();
}

std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> Socket::recv() {
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers =
        device->getAssociatedTCPconnectionOrNull(this)
            ->waitForApplicationData();
    return layers;
}


void Socket::send(std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                      applicationLayers) {
    device->getAssociatedTCPconnectionOrNull(this)->sendApplicationData(
        std::move(applicationLayers));
}