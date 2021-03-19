#ifndef BGPSIMULATION_ENTITIES_ROUTER_H
#define BGPSIMULATION_ENTITIES_ROUTER_H

#include <string>

#include "Connection.h"
#include "Device.h"
#include "Network_details.h"

using namespace std;

class Router : public virtual Device {
   public:
    string                     AS_number;
    vector<Network_details *> *network_details_list;
    // TODO: announced_prefixes
    // TODO: local_preferences
    // TODO: trust

    Router(string                     ID,
           string                     AS_number,
           vector<Network_details *> *network_details_list)
        : Device(std::move(ID)),
          AS_number(std::move(AS_number)),
          network_details_list(network_details_list) {}

    ~Router() override {
        for (auto network_details : *network_details_list) {
            delete network_details;
        }
        delete network_details_list;
    }
};

#endif  // BGPSIMULATION_ENTITIES_ROUTER_H
