#ifndef BGP_SIMULATION_ENTITIES_CLIENT_H
#define BGP_SIMULATION_ENTITIES_CLIENT_H

#include <string>
#include <vector>

#include "Device.h"
#include "NetworkCard.h"

using namespace std;

class Client : public virtual Device {
   public:
    Client(string                 ID,
           string                 defaultGateway,
           vector<NetworkCard *> *networkCards)
        : Device(std::move(ID), defaultGateway, networkCards) {}

    ~Client() override = default;
};

#endif  // BGP_SIMULATION_ENTITIES_CLIENT_H
