#ifndef BGPSIMULATION_SOCKET_SOCKET_H
#define BGPSIMULATION_SOCKET_SOCKET_H

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <stack>
#include <string>

#include "IpAddress.h"
#include "Layer.h"

// forward declarations
#include "../entities/Device.fwd.h"
#include "../tcp/TCPConnection.fwd.h"


class Socket {
   public:
    Device* device;

    bool        running = false;
    std::string name    = "Socket";

    std::shared_ptr<TCPConnection> tcpConnection = nullptr;

    std::mutex              tcpConnection_mutex;
    std::condition_variable tcpConnection_wakeup;


    Socket(Device* device);
    Socket(Device* device, std::shared_ptr<TCPConnection> tcpConnection);

    ~Socket();

    /**
     * Bind the socket to a source address and a source port
     * @param srcAddr the source address
     * @param srcPort the source port
     * @return 0 if the bind was successful
     */
    int bind(const pcpp::IPv4Address& srcAddr, const uint16_t& srcPort);

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

#endif  // BGPSIMULATION_SOCKET_SOCKET_H
