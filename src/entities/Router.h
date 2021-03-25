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

    Router(string                 ID,
           string                 AS_number,
           pcpp::IPv4Address      defaultGateway,
           vector<NetworkCard *> *networkCards)
        : Device(std::move(ID), defaultGateway, networkCards),
          AS_number(std::move(AS_number)) {}

    ~Router() override = default;
};

#endif  // BGPSIMULATION_ENTITIES_ROUTER_H
