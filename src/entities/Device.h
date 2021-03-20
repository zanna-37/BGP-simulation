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
    vector<Link *>         links;

    Device(string ID, vector<NetworkCard *> *networkCards)
        : ID(std::move(ID)), networkCards(networkCards) {}

    virtual ~Device() {
        while (links.begin() != links.end()) {
            auto it = links.begin();  // this is safe even if the vector changes
            this->disconnectFrom(*it);
        }
        for (auto networkCard : *networkCards) {
            delete networkCard;
        }
        delete networkCards;
    }

    void disconnectFrom(Link *linkToDisconnect) {
        vector<Link *>::iterator elementToDelete;
        for (auto it = links.begin(); it != links.end(); it++) {
            if (*it == linkToDisconnect) {
                elementToDelete = it;
            }
        }
        Device *peer = linkToDisconnect->getPeerOrNull(this);

        linkToDisconnect->disconnect(this);
        links.erase(elementToDelete);

        if (peer != nullptr && this != peer) {  // peer is null if it has
                                                // already been disconnected
            peer->disconnectFrom(linkToDisconnect);
        }
    }
};


#endif  // BGPSIMULATION_ENTITIES_DEVICE_H
