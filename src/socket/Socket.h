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
     * Establish the TCP connection and return a new socket with the references
     * of the new connection
     * @return a socket with the properties of the newly established connection
     */
    Socket* accept();

    /**
     * Client active connection, accept a destination address and port and
     * return 0 if the socket has been established
     * @param dstAddr the destination address
     * @param dstPort the destination port
     * @return 0 if the connection has been established
     */
    int connect(const pcpp::IPv4Address& dstAddr, uint16_t dstPort);

    /**
     * send data the socket by using the TCP connection established
     * @param applicationLayers the applicaiton layers to be sent through the
     * TCP connection
     */
    void send(std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                  applicationLayers);

    /**
     * Blocking function that waits for new data to be received in the TCP
     * connection queue and fill the application layers given.
     * @param layers The application layers that are filled with the received
     * data, ready to be passed to the application. Always pass an empty object.
     * In case of errors the content of this variable is not defined.
     *
     * @return 0 if successful or -1 in case of errors.
     */
    [[nodiscard]] int recv(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>& layers);

    void close();
};

#endif  // BGPSIMULATION_SOCKET_SOCKET_H
