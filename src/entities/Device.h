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

    NetworkCard *getNetworkCardByInterfaceOrNull(
        const string &interfaceToSearch);
};


#endif  // BGPSIMULATION_ENTITIES_DEVICE_H
