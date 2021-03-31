#ifndef BGP_SIMULATION_ENTITIES_ENDPOINT_H
#define BGP_SIMULATION_ENTITIES_ENDPOINT_H

#include <string>
#include <vector>

#include "Device.h"
#include "NetworkCard.h"

using namespace std;

class EndPoint : public virtual Device {
   public:
    EndPoint(string                 ID,
           pcpp::IPv4Address      defaultGateway,
           vector<NetworkCard *> *networkCards)
        : Device(std::move(ID), defaultGateway, networkCards) {}

    ~EndPoint() override = default;
};

#endif  // BGP_SIMULATION_ENTITIES_ENDPOINT_H
