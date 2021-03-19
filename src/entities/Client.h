#ifndef BGP_SIMULATION_ENTITIES_CLIENT_H
#define BGP_SIMULATION_ENTITIES_CLIENT_H

#include <string>

#include "Connection.h"
#include "Device.h"
#include "Network_details.h"

using namespace std;

class Client : public virtual Device {
   public:
    vector<Network_details *> *network_details_list;

    Client(string ID, vector<Network_details *> *network_details_list)
        : Device(std::move(ID)), network_details_list(network_details_list) {}

    ~Client() override {
        for (auto network_details : *network_details_list) {
            delete network_details;
        }
        delete network_details_list;
    }
};

#endif  // BGP_SIMULATION_ENTITIES_CLIENT_H
