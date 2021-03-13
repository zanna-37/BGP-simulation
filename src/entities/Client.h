#ifndef BGP_SIMULATION_ENTITIES_CLIENT_H
#define BGP_SIMULATION_ENTITIES_CLIENT_H

#include <string>

#include "Connection.h"
#include "Device.h"

using namespace std;

class Client : public Device {
   public:
    Client(string ID, Connection *connection)
        : Device(std::move(ID)), connection(connection) {}

    Connection *connection;
};

#endif  // BGP_SIMULATION_ENTITIES_CLIENT_H
