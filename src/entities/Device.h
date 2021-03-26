#ifndef BGPSIMULATION_ENTITIES_DEVICE_H
#define BGPSIMULATION_ENTITIES_DEVICE_H

#include <IPv4Layer.h>
#include <Packet.h>

#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "../ip/RoutingTable.h"
#include "../ip/TableRow.h"
#include "NetworkCard.h"

using namespace std;


class NetworkCard;  // forward declaration

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
    vector<NetworkCard *> *networkCards;
    std::thread *          deviceThread = nullptr;
    RoutingTable           routingTable;
    bool                   running;

    Device(string ID, pcpp::IPv4Address defaultGateway);


    virtual ~Device() {
        running = false;
        // deviceThread->join();
        for (NetworkCard *networkCard : *networkCards) {
            delete networkCard;
        }
        delete networkCards;
        delete deviceThread;
    }

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
    void receiveMessage(std::string data);

    void forwardMessage(std::string data);

    void sendPacket(pcpp::Packet *packet, NetworkCard *networkCard);

    void receivePacket(pcpp::Packet *packet);
};


#endif  // BGPSIMULATION_ENTITIES_DEVICE_H
