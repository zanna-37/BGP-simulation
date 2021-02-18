#ifndef BGPSIMULATION_ENTITIES_CONNECTION_H
#define BGPSIMULATION_ENTITIES_CONNECTION_H

#include <string>

#include "./Device.h"

using namespace std;

enum connection_status { active, failed };

class Connection {
   public:
    Connection(string            source_interface,
               string            source_IP,
               string            destination_IP,
               connection_status conn_status)
        : source_interface(std::move(source_interface)),
          source_IP(std::move(source_IP)),
          destination_IP(std::move(destination_IP)),
          conn_status(conn_status) {}

    string            AS_number;
    string            source_interface;
    string            source_IP;
    string            destination_IP;
    connection_status conn_status;
};

#endif  // BGPSIMULATION_ENTITIES_CONNECTION_H