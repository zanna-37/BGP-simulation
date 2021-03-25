#ifndef BGPSIMULATION_ENTITIES_NET_DETAILS_H
#define BGPSIMULATION_ENTITIES_NET_DETAILS_H

#include <IPv4Layer.h>

#include <memory>
#include <string>

#include "Link.h"

using namespace std;

class Link;  // forward declaration
class NetworkCard {
   public:
    string            netInterface;
    pcpp::IPv4Address IP;
    pcpp::IPv4Address netmask;
    shared_ptr<Link>  link;

    NetworkCard(string            netInterface,
                pcpp::IPv4Address IP,
                pcpp::IPv4Address netmask)
        : netInterface(std::move(netInterface)),
          IP(std::move(IP)),
          netmask(std::move(netmask)) {}

    /**
     * Connect the networkCard to the specified link.
     * It does nothing if the networkCard is already connected to a link.
     *
     * @param linkToConnect The link to connect to.
     */
    void connect(const shared_ptr<Link>& linkToConnect);

    /**
     * Disconnect the networkCard from the specified link.
     * It does nothing if the networkCard is not connected to the specified link
     * or no link at all.
     *
     * @param linkToConnect The link to disconnect from.
     */
    void disconnect(const shared_ptr<Link>& linkToDisconnect);
};

#endif  // BGPSIMULATION_ENTITIES_NET_DETAILS_H
