#ifndef SOCKET_H
#define SOCKET_H

#include <IPv4Layer.h>
#include <Layer.h>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <stack>
#include <string>
#include <thread>

#include "../entities/Device.h"
#include "../tcp/TCPConnection.h"


// forward declarations
#include "../entities/Device.fwd.h"
#include "../tcp/TCPConnection.fwd.h"


class Socket {
   public:
    /**
     * Network protocol domain to choose between. We implement only IP
     */
    enum Domain {
        AF_INET
        // other Socket domain
    };
    /**
     * The type of socket, it can be connectionful or connectionless. We
     * implement only the socket over TCP protocol, meaning SOCK_STREAM
     */
    enum Type { SOCK_STREAM, SOCK_DGRAM };


    int     domain;
    int     type;
    Device* device;

    // application data receiving queue
    std::queue<std::stack<pcpp::Layer*>*> receivingQueue;
    std::mutex                            receivingQueue_mutex;
    std::condition_variable               receivingQueue_wakeup;
    std::thread*                          receivingThread = nullptr;

    bool         running      = false;
    std::thread* socketThread = nullptr;
    std::string  name         = "Socket";

    pcpp::IPv4Address srcAddr = pcpp::IPv4Address::Zero;
    pcpp::IPv4Address dstAddr = pcpp::IPv4Address::Zero;


    uint16_t srcPort = 0;
    uint16_t dstPort = 0;

    std::mutex              tcpConnection_mutex;
    std::condition_variable tcpConnection_wakeup;

    // Application connection
    BGPConnection* applicationConnection = nullptr;


    Socket(int domain, int type);
    ~Socket();

    int     bind(const pcpp::IPv4Address& srcAddr, uint16_t srcPort);
    int     listen();
    Socket* accept();

    int connect(const pcpp::IPv4Address& dstAddr, uint16_t dstPort);

    void                      send(std::stack<pcpp::Layer*>* applicationLayers);
    std::stack<pcpp::Layer*>* recv();

    void enqueueApplicationLayers(std::stack<pcpp::Layer*>* layers);

    void dataArrived();
};
#endif