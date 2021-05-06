#include "Socket.h"

#include "../logger/Logger.h"

Socket::Socket(int type, int domain) : type(type), domain(domain) {
    running         = true;
    receivingThread = new std::thread([&]() {
        while (running) {
            std::unique_lock<std::mutex> receivingQueue_uniquLock(
                receivingQueue_mutex);

            while (receivingQueue.empty() && running) {
                receivingQueue_wakeup.wait(receivingQueue_uniquLock);

                if (receivingQueue.empty() && running) {
                    L_DEBUG(name, "Spurious wakeup");
                }
            }

            if (running) {
                std::stack<pcpp::Layer*>* applicationLayers =
                    receivingQueue.front();
                receivingQueue.pop();
                applicationConnection->processMessage(applicationLayers);
            } else {
                L_DEBUG(name, "Shutting Down");
            }
        }
    });
}

Socket::~Socket() {
    running = false;

    std::unique_lock<std::mutex> receivingQueue_uniqueLock(
        receivingQueue_mutex);

    receivingQueue_wakeup.notify_one();
    receivingQueue_uniqueLock.unlock();
}
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

std::stack<pcpp::Layer*>* Socket::recv() {
    std::unique_lock<std::mutex> receivingQueue_uniqueLock(
        receivingQueue_mutex);
    while (receivingQueue.empty()) {
        receivingQueue_wakeup.wait(receivingQueue_uniqueLock);

        if (receivingQueue.empty()) {
            L_DEBUG(name, "Spurious wakeup");
        }
    }

    std::stack<pcpp::Layer*>* layers = receivingQueue.front();
    receivingQueue.pop();

    return layers;
}


void Socket::enqueueApplicationLayers(std::stack<pcpp::Layer*>* layers) {
    std::unique_lock<std::mutex> receivingQueue_uniqueLock(
        receivingQueue_mutex);
    receivingQueue.push(layers);
    receivingQueue_wakeup.notify_one();
    receivingQueue_uniqueLock.unlock();
}