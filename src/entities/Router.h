#ifndef BGPSIMULATION_ENTITIES_ROUTER_H
#define BGPSIMULATION_ENTITIES_ROUTER_H

#include <string>

#include "Device.h"
#include "Link.h"
#include "Network_details.h"

using namespace std;

class Router : public virtual Device {
   public:
    string AS_number;
    // TODO: announced_prefixes
    // TODO: local_preferences
    // TODO: trust

    Router(string                     ID,
           string                     AS_number,
           vector<Network_details *> *network_details_list)
        : Device(std::move(ID), network_details_list),
          AS_number(std::move(AS_number)) {}

    ~Router() override = default;
};

#endif  // BGPSIMULATION_ENTITIES_ROUTER_H
