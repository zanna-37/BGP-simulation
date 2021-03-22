#ifndef BGPSIMULATION_ENTITIES_DEVICE_H
#define BGPSIMULATION_ENTITIES_DEVICE_H

#include <string>
#include <vector>

#include "NetworkCard.h"

using namespace std;


class NetworkCard;  // forward declaration
class Device {
   public:
    string                 ID;
    vector<NetworkCard *> *networkCards;

    Device(string ID, vector<NetworkCard *> *networkCards)
        : ID(std::move(ID)), networkCards(networkCards) {}

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
