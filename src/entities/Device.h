#ifndef BGPSIMULATION_ENTITIES_DEVICE_H
#define BGPSIMULATION_ENTITIES_DEVICE_H

#include <iostream>
#include <string>
#include <vector>

#include "Link.h"
#include "Network_details.h"
using namespace std;

class Device {
   public:
    string                     ID;
    vector<Network_details *> *network_details_list;
    vector<Link *>             links;

    Device(string ID, vector<Network_details *> *network_details_list)
        : ID(std::move(ID)), network_details_list(network_details_list) {}

    virtual ~Device() {
        while (links.begin() != links.end()) {
            auto it = links.begin();  // this is safe even if the vector changes
            this->disconnectFrom(*it);
        }
        for (auto network_details : *network_details_list) {
            delete network_details;
        }
        delete network_details_list;
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
