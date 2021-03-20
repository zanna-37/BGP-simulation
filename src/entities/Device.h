#ifndef BGPSIMULATION_ENTITIES_DEVICE_H
#define BGPSIMULATION_ENTITIES_DEVICE_H

#include <iostream>
#include <string>
#include <vector>

#include "Link.h"
#include "NetworkCard.h"
using namespace std;

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

    NetworkCard *getNetworkCardByInterfaceOrNull(string interfaceToSearch) {
        for (const auto &networkCard : *networkCards) {
            if (networkCard->net_interface == interfaceToSearch) {
                return networkCard;
            }
        }
        return nullptr;
    }
};


#endif  // BGPSIMULATION_ENTITIES_DEVICE_H
