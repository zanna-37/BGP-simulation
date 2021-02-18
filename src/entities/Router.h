#ifndef BGPSIMULATION_ENTITIES_ROUTER_H
#define BGPSIMULATION_ENTITIES_ROUTER_H

#include <string>

#include "Connection.h"
#include "Device.h"

using namespace std;

class Router : public Device {
   public:
    Router(string ID, string AS_number, Connection* connections)
        : Device(std::move(ID)), AS_number(std::move(AS_number)), connections(connections) {}

    string      AS_number;
    Connection* connections;
    // TODO: announced_prefixes
    // TODO: local_preferences
    // TODO: trust
};

#endif  // BGPSIMULATION_ENTITIES_ROUTER_H
