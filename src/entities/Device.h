#ifndef BGPSIMULATION_ENTITIES_DEVICE_H
#define BGPSIMULATION_ENTITIES_DEVICE_H

#include <string>
#include <utility>
#include <vector>

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

    Device(string                 ID,
           pcpp::IPv4Address      defaultGateway,
           vector<NetworkCard *> *networkCards)
        : ID(std::move(ID)),
          defaultGateway(defaultGateway),
          networkCards(networkCards) {}

    virtual ~Device() {
        for (auto networkCard : *networkCards) {
            delete networkCard;
        }
        delete networkCards;
    }

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
};


#endif  // BGPSIMULATION_ENTITIES_DEVICE_H
