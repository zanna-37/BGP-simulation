#ifndef BGPSIMULATION_ENTITIES_NET_DETAILS_H
#define BGPSIMULATION_ENTITIES_NET_DETAILS_H

#include <string>


using namespace std;

class Network_details {
   public:
    Network_details(string net_interface,
                    string IP,
                    string netmask,
                    string default_gateway)
        : net_interface(std::move(net_interface)),
          IP(std::move(IP)),
          netmask(std::move(netmask)),
          default_gateway(std::move(default_gateway)) {}

    string net_interface;
    string IP;
    string netmask;
    string default_gateway;
};

#endif  // BGPSIMULATION_ENTITIES_NET_DETAILS_H
