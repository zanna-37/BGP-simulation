#ifndef BGP_SIMULATION_ENTITIES_CLIENT_H
#define BGP_SIMULATION_ENTITIES_CLIENT_H

#include <string>

#include "Device.h"
#include "Link.h"
#include "Network_details.h"

using namespace std;

class Client : public virtual Device {
   public:
    Client(string ID, vector<Network_details *> *network_details_list)
        : Device(std::move(ID), network_details_list) {}

    ~Client() override = default;
};

#endif  // BGP_SIMULATION_ENTITIES_CLIENT_H
