#ifndef BGPSIMULATION_ENTITIES_DEVICE_H
#define BGPSIMULATION_ENTITIES_DEVICE_H

#include <IPv4Layer.h>
#include <Packet.h>
#include <TcpLayer.h>

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <stack>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "../ip/TableRow.h"
#include "../socket/Socket.h"
#include "../tcp/TCPConnection.h"
#include "NetworkCard.h"

// forward declarations
#include "../bgp/BGPConnection.fwd.h"
#include "../ip/TableRow.fwd.h"
#include "../socket/Socket.fwd.h"
#include "../tcp/TCPConnection.fwd.h"
#include "NetworkCard.fwd.h"

using namespace std;

/**
 * This class abstracts the concept of a general network device
 * \verbatim
 *                                +----------+
 *                         + -----| Endpoint |
 *  +------------+         |      +----------+
 *  |   Device   | --------+
 *  | (abstract) |         |
 *  +------------+         |      +----------+
 *                         + -----|  Router  |
 *                                +----------+
 * \endverbatim
 */
class Device {
   public:
    /**
     * Internal reference that uniquely identifies a component in the
     * simulation.
     */
    std::string ID;

    /**
     * The default gateway of a device. It can be unset, it that case its
     * value is \a pcpp::IPv4Address::Zero.
     */
    pcpp::IPv4Address defaultGateway;

    /**
     * A list of all the network interfaces of a device.
     */
    std::vector<NetworkCard *> *networkCards = nullptr;

    /**
     * A std::vector used to implement the ip routing table. Each row is a
     * TableRow object
     */
    std::vector<TableRow> routingTable = std::vector<TableRow>();

    /**
     * The hashmap containing the existing TCPConnections. When a device
     * connects or receives a SYN, it istanzitate a new TCP connection that is
     * stored here.
     */
    // std::map<std::size_t, TCPConnection *> tcpConnections;
    std::vector<std::weak_ptr<TCPConnection>>
        tcpConnections;  // TODO put in a map with the port

    /**
     * Threads that manage inbound network communication.
     */
    std::vector<std::thread> netInputThreads;

    /**
     * A list of listening socket in the device. According to the
     * linux implementation, listening sockets are never used as connected
     * sockets, since accept return a new socket. We gather all the listening
     * sockets in this vector
     */
    std::vector<Socket *> listeningSockets;  // TODO probalby to remove

    Device(std::string ID, pcpp::IPv4Address defaultGateway);

    virtual ~Device();

    /**
     * Method used by the parser to populate the network cards vector. The
     * constructor does not populate this because of circular dependencies
     * (Network Card has its owner).
     * @warning this method should be called only by the parser, after the
     * vector is populated
     * @param networkCards the network cards vector
     */
    void addCards(std::vector<NetworkCard *> *networkCards);

    /**
     * Get the networkCard associated with the specified interface.
     *
     * @param interfaceToSearch Name of the interface to search in this device.
     * @example \code NetworkCard *networkCard =
     * device.getNetworkCardByInterfaceOrNull("eth0"); \endcode
     * @return The \a NetworkCard object associated with the specified interface
     * or \a nullptr if the interface is not found on the device.
     */
    NetworkCard *getNetworkCardByInterfaceOrNull(
        const std::string &interfaceToSearch) const;

    /**
     * Boot up the device. It simulate the turning on of the device, where the
     * thread is created and the device waits for receiving packets. It creates
     * the Routing table by reading through the network cards vector. If the
     * network card has a valid default Gateway, it also create the default
     * route TableRow.
     */
    void bootUp();

    /**
     * Process a packet (std::stack of layers) when it arrives. It handles new
     * and existing TCPConnections.
     * @param layers the std::stack simulating the packet (TCP and IP layers)
     */
    void processMessage(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers);

    /**
     * Forward the message to the next destination, if the device is a router,
     * or drops the packet if it is an endpoint
     * @param layers the std::stack simulating the packet
     * @param networkCard the network card responsible to forward the message
     */
    virtual void forwardMessage(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers,
        NetworkCard *networkCard) = 0;

    /**
     * Creates the IP layer, find the next hop and send the packet to the layer
     * below (networkCard)
     * @param layers the std::stack simulating the packet
     * @param dstAddr the destination address of the packet
     */
    void sendPacket(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers,
        const pcpp::IPv4Address &                                 dstAddr);

    shared_ptr<TCPConnection> getExistingTcpConnectionOrNull(
        const pcpp::IPv4Address &srcAddr,
        const uint16_t &         srcPort,
        const pcpp::IPv4Address &dstAddr,
        const uint16_t &         dstPort);

    int bind(const shared_ptr<TCPConnection> &tcpConnection);

    uint16_t getFreePort();

    NetworkCard *getNextHopNetworkCardOrNull(
        const pcpp::IPv4Address &dstAddr) const;

   protected:
    /**
     * Subclasses hook to perform initialization logic.
     * This is called at the end of \a Device::bootUp().
     */
    virtual void bootUpInternal() = 0;

    void shutdown();

   private:
    /**
     * A bool indicating if the device is running or not. \a Device::bootUp()
     * starts the device and sets it to \a true. \a Device::~Device() set it to
     * \a false.
     */
    std::atomic<bool> running{false};
    std::mutex        ports_mutex;
};

#endif  // BGPSIMULATION_ENTITIES_DEVICE_H
