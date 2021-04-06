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

#include "../ip/TableRow.h"
#include "../tcp/TCPConnection.h"
#include "NetworkCard.h"

// forward declarations
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
    vector<NetworkCard *> *  networkCards = nullptr;
    std::thread *            deviceThread = nullptr;
    std::vector<TableRow *> *routingTable = nullptr;
    bool                     running;

    std::mutex receivedPacketsEventQueue_mutex;  // mutex to lock the queue
    std::condition_variable
        receivedPacketsEventQueue_wakeup;  // condition variable set when the
                                           // queue is not empty
    queue<ReceivedPacketEvent *> receivedPacketsEventQueue;  // queue of events


    std::map<std::size_t, TCPConnection *> tcpConnections;

    TCPConnection *listenConnection = nullptr;

    Device(string ID, pcpp::IPv4Address defaultGateway);


    virtual ~Device();

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

    void start();
    void processMessage(stack<pcpp::Layer *> *layers);

    virtual void forwardMessage(stack<pcpp::Layer *> *layers,
                                NetworkCard *         networkCard) = 0;

    void sendPacket(stack<pcpp::Layer *> *layers, std::string dstAddr);

    void receivePacket(stack<pcpp::Layer *> *layers, NetworkCard *origin);

    void enqueueEvent(ReceivedPacketEvent *event);

    void listen();
    void connect(pcpp::IPv4Address *dstAddr, uint16_t dstPort);

    TCPConnection *getExistingConnectionOrNull(pcpp::IPv4Layer *ipLayer,
                                               pcpp::TcpLayer * tcpLayer);

    void addTCPConnection(TCPConnection *connection);

    NetworkCard *findNextHop(pcpp::IPv4Address *dstAddress);
    void         printTable();

   private:
    void        printElement(std::string t);
    std::size_t tcpConnectionHash(std::string dstAddr, uint16_t dstPort);
};

#endif  // BGPSIMULATION_ENTITIES_DEVICE_H
