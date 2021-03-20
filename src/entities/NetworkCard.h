#ifndef BGPSIMULATION_ENTITIES_NET_DETAILS_H
#define BGPSIMULATION_ENTITIES_NET_DETAILS_H

#include <memory>

#include "Link.h"

using namespace std;

class Link;  // forward declaration
class NetworkCard {
   public:
    string           net_interface;
    string           IP;
    string           netmask;
    string           default_gateway;
    shared_ptr<Link> link;

    NetworkCard(string net_interface,
                string IP,
                string netmask,
                string default_gateway)
        : net_interface(std::move(net_interface)),
          IP(std::move(IP)),
          netmask(std::move(netmask)),
          default_gateway(std::move(default_gateway)) {}


    void connect(shared_ptr<Link> linkToConnect);

    void disconnect(shared_ptr<Link> linkToDisconnect);
};

#endif  // BGPSIMULATION_ENTITIES_NET_DETAILS_H
