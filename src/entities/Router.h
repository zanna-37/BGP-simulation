#ifndef BGPSIMULATION_ENTITIES_ROUTER_H
#define BGPSIMULATION_ENTITIES_ROUTER_H

#include <string>

#include "Device.h"
#include "NetworkCard.h"

using namespace std;

class Router : public virtual Device {
   public:
    string AS_number;
    // TODO: announced_prefixes
    // TODO: local_preferences
    // TODO: trust

    Router(string ID, string AS_number, vector<NetworkCard *> *networkCards)
        : Device(std::move(ID), networkCards),
          AS_number(std::move(AS_number)) {}

    ~Router() override = default;
};

#endif  // BGPSIMULATION_ENTITIES_ROUTER_H
