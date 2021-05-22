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

    bool        running = false;
    std::string name    = "Socket";

    pcpp::IPv4Address srcAddr = pcpp::IPv4Address::Zero;
    pcpp::IPv4Address dstAddr = pcpp::IPv4Address::Zero;

    uint16_t srcPort = 0;
    uint16_t dstPort = 0;

    std::mutex              tcpConnection_mutex;
    std::condition_variable tcpConnection_wakeup;


    Socket(int domain, int type);

    ~Socket() = default;

    /**
     * Bind the socket to a source address and a source port
     * @param srcAddr the source address
     * @param srcPort the source port
     * @return 0 if the bind was successful
     */
    int bind(const pcpp::IPv4Address& srcAddr, uint16_t srcPort);

    /**
     * listen for incoming TCP connections
     * @return 0 if there is a TCP connection pending
     */
    int listen();

    /**
     * Enstablish the TCP connection and return a new socket with the references
     * of the new connection
     * @return a socket with the properties of the newly enstablished connection
     */
    Socket* accept();

    /**
     * Client active connection, accept a destination address and port and
     * return 0 if the socket has been enstablished
     * @param dstAddr the destination address
     * @param dstPort the destination port
     * @return 0 if the connection has been enstablished
     */
    int connect(const pcpp::IPv4Address& dstAddr, uint16_t dstPort);

    /**
     * send data the socket by using the TCP connection enstablished
     * @param applicationLayers the applicaiton layers to be sent through the
     * TCP connection
     */
    void send(std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                  applicationLayers);
    /**
     * Wait for new data to be received in the TCP connection queue and return
     * the application layers, processed by the application
     * @return a stack of application layers
     */
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> recv();


    /**
     * unlocks the condition variable and break out of the loop. It is used by
     * the listening thread to return when a TCP connection is pending
     */
    void dataArrived();
    void close();
};
#endif