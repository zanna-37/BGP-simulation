#ifndef BGPSIMULATION_ENTITIES_LINK_H
#define BGPSIMULATION_ENTITIES_LINK_H

#include <string>

#include "Device.h"


using namespace std;

enum Connection_status { active, failed };

class NetworkCard;  // forward declaration
class Link {
   public:
    pair<NetworkCard *, NetworkCard *> device_source_networkCards;
    Connection_status                  connection_status;

    Link(Connection_status connection_status)
        : connection_status(connection_status) {}

    void disconnect(NetworkCard *networkCard) {
        if (device_source_networkCards.first == networkCard) {
            device_source_networkCards.first = nullptr;
        } else if (device_source_networkCards.second == networkCard) {
            device_source_networkCards.second = nullptr;
        } else {
            cout
                << "[-] This link is not connected to the specified networkCard"
                << endl;  // TODO ERROR
        }
    }

    void connect(NetworkCard *networkCard) {
        if (device_source_networkCards.first == nullptr) {
            device_source_networkCards.first = networkCard;
        } else if (device_source_networkCards.second == nullptr) {
            device_source_networkCards.second = networkCard;
        } else {
            cout << "[-] This link is already fully connected"
                 << endl;  // TODO ERROR
        }
    }

    NetworkCard *getPeerNetworkCardOrNull(NetworkCard *networkCard) {
        if (device_source_networkCards.first == networkCard) {
            return device_source_networkCards.second;
        } else if (device_source_networkCards.second == networkCard) {
            return device_source_networkCards.first;
        } else {
            return nullptr;
        }
    }
};


#endif  // BGPSIMULATION_ENTITIES_LINK_H
