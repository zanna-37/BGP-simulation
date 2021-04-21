#ifndef BGPSIMULATION_ENTITIES_DEVICE_H
#define BGPSIMULATION_ENTITIES_DEVICE_H

#include <IPv4Layer.h>
#include <Packet.h>
#include <TcpLayer.h>

#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <stack>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "../bgp/BGPConnection.h"
#include "../ip/TableRow.h"
#include "../tcp/TCPConnection.h"
#include "NetworkCard.h"

// forward declarations
#include "../bgp/BGPConnection.fwd.h"
#include "../ip/TableRow.fwd.h"
#include "../tcp/TCPConnection.fwd.h"
#include "NetworkCard.fwd.h"

using namespace std;

class ReceivedPacketEvent {
   public:
    NetworkCard *networkCard;

    enum Description { PACKET_ARRIVED };
    Description description;
    ReceivedPacketEvent(NetworkCard *networkCard, Description description);
    ~ReceivedPacketEvent() {}
};

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
    string ID;

    /**
     * The default gateway of a device. It can be unset, it that case its
     * value is \a pcpp::IPv4Address::Zero.
     */
    pcpp::IPv4Address defaultGateway;

    /**
     * A list of all the network interfaces of a device.
     */
    vector<NetworkCard *> *networkCards = nullptr;
    /**
     * the pointer to the device thread. Each device works on a separate thread
     * that handle the packet received and send packets to other devices
     */
    std::thread *deviceThread = nullptr;
    /**
     * A std::vector used to implement the ip routing table. Each row is a
     * TableRow object
     */
    std::vector<TableRow *> *routingTable = nullptr;
    /**
     * A bool indicating if the device is running or not. Device::start() start
     * the deivce and sets it to true
     *
     */
    bool running;

    /**
     * Mutex to lock the queue
     */
    std::mutex receivedPacketsEventQueue_mutex;
    /**
     * condition variable set when the queue is not empty
     */
    std::condition_variable receivedPacketsEventQueue_wakeup;
    /**
     * Queue of events. An event is enqueued when a packet arrives at the
     * network card packet queue
     */
    queue<ReceivedPacketEvent *> receivedPacketsEventQueue;

    /**
     * The hashmap containing the existing TCPConnections. When a device
     * connects or receives a SYN, it istanzitate a new TCP connection that is
     * stored here.
     */
    std::map<std::size_t, TCPConnection *> tcpConnections;


    std::map<std::string, BGPConnection *> bgpConnections;


    std::map<TCPConnection *, BGPConnection *> applicationSockets;

    /**
     * If a deivice has open ports and accept connections, this pointer is
     * initialized
     * (// TODO change it to a list of connection, one for each open port)
     */
    TCPConnection *listenConnection = nullptr;

    Device(string ID, pcpp::IPv4Address defaultGateway);


    virtual ~Device();

    /**
     * Method used by the parser to populate the network cards vector. The
     * constructor does not populate this because of circular dependencies
     * (Network Card has its owner).
     * @warning this method should be called only by the parser, after the
     * vector is populated
     * @param networkCards the network cards vector
     */
    void addCards(vector<NetworkCard *> *networkCards);

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
        const string &interfaceToSearch);

    /**
     * Start the device. It simulate the turning on of the device, where the
     * thread is created and the device waits for receiving packets. It creates
     * the Routing table by reading through the network cards vector. If the
     * network card has a valid default Gateway, it also create the default
     * route TableRow
     */
    void start();

    /**
     * Process a packet (std::stack of layers) when it arrives. It handles new
     * and existing TCPConnections.
     * @param layers the std::stack simulating the packet (TCP and IP layers)
     */
    void processMessage(stack<pcpp::Layer *> *layers);

    /**
     * Forward the message to the next destination, if the device is a router,
     * or drops the packet if it is an endpoint
     * @param layers the std::stack simulating the packet
     * @param networkCard the network card responsible to forward the message
     */
    virtual void forwardMessage(stack<pcpp::Layer *> *layers,
                                NetworkCard *         networkCard) = 0;

    /**
     * Creates the IP layer, find the next hop and send the packet to the layer
     * below (networkCard)
     * @param layers the std::stack simulating the packet
     * @param dstAddr the destination address of the packet
     */
    void sendPacket(stack<pcpp::Layer *> *layers, std::string dstAddr);


    /**
     * Enqueue an receivedPacket event. It is used to handle different network
     * cards packets queue in an organized manner.
     * @param event a packet has arrivet at one of the device's network card
     */
    void enqueueEvent(ReceivedPacketEvent *event);

    /**
     * A server that wants to start listening open a port (// TODO not only 179)
     * and instantiate a listening connection, waiting for messages to arrive.
     */
    void listen();

    /**
     * Create a new TCP Connection and connect the device to the destination
     * specified. It also adds the newly created connection to tcpConnections
     * hashmap
     * @param dstAddr, a pointer to the destination address
     * @param dstPort the destination port the device wants to connect to
     * @return the TCP connection created
     */
    TCPConnection *connect(std::string dstAddr, uint16_t dstPort);

    /**
     * Gently close the connection using 4-way closing mechanism, following TCP
     * RFC 793
     * @param dstAddr, a pointer to the destination address (
     * @param dstPort the destination port
     */
    void closeConnection(std::string dstAddr, uint16_t dstPort);

    /**
     * Force close the connection, sending a RST packet. Faster way to close a
     * connection or used in case of errors
     * @param dstAddr, a pointer to the destination address
     * @param dstPort the destination port
     */
    void resetConnection(std::string dstAddr, uint16_t dstPort);

    /**
     * Search for existing connections in tcpConnections hashmap. If no
     * connection exists, or the connection is in state CLOSED, a \a nullptr is
     * returned
     * @param address the destination address of the connection
     * @param port the destination port of the connection
     * @return the existing TCPConnection or a \a nullptr
     */
    TCPConnection *getExistingConnectionOrNull(std::string address,
                                               uint16_t    port);


    /**
     * Add a TCPConnection to the tcpConnections hashmap
     * @param connection the TCPConnection to add
     */
    void addTCPConnection(TCPConnection *connection);
    /**
     * Remove a TCP connection from the hashmap
     * @param connection the TCPConnection to remove
     */
    void removeTCPConnection(TCPConnection *connection);

    void addBGPConnection(BGPConnection connection);

    void removeBGPConnection(BGPConnection connection);

    /**
     * Search for the network card that have to send the packet, based on the IP
     * destination address. If no network card can handle the message, a \a
     * nullptr is returned
     * @param dstAddress the destnation address of the packet
     */
    NetworkCard *findNextHop(pcpp::IPv4Address *dstAddress);

    /**
     * Print the routing table, using <iomanip> library
     */
    void printTable();

    /**
     * Handle the application layer of the packet. It depends on the application
     * ports open
     * @param layers the stack containing the application layers
     * @param port the application port
     */
    void handleApplicationLayer(std::stack<pcpp::Layer *> *layers,
                                TCPConnection *            connection);


    void bgpConnect(std::string dstAddr);

   private:
    /**
     * private methd to print a single element in the routing table
     * @param t the string to beautify print
     */
    void printElement(std::string t);

    /**
     * Compute the hash of the connection based on the destination IP address
     * and the destination port
     * @param dstAddr string of the IP address
     * @param dstPort the destination port
     * @return the hash
     */
    std::size_t tcpConnectionHash(std::string dstAddr, uint16_t dstPort);

    /**
     * Receive a packet from one of its network cards. It checks if the message
     * if for this router or it must forward (or drop) it
     * @warning it should be called by the network card
     * @param layers the std::stack simulating the packet.
     * @param origin the network card that received the packet
     */
    void receivePacket(stack<pcpp::Layer *> *layers, NetworkCard *origin);

    friend class NetworkCard;
};

#endif  // BGPSIMULATION_ENTITIES_DEVICE_H
