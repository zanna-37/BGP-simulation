#ifndef BGP_SIMULATION_ENTITIES_CLIENT_H
#define BGP_SIMULATION_ENTITIES_CLIENT_H

#include <string>

#include "Device.h"
#include "Link.h"
#include "NetworkCard.h"

using namespace std;

class Client : public virtual Device {
   public:
    Client(string ID, vector<NetworkCard *> *networkCards)
        : Device(std::move(ID), networkCards) {}

    ~Client() override = default;
};

#endif  // BGP_SIMULATION_ENTITIES_CLIENT_H
