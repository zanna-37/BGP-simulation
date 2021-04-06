#ifndef BGPSIMULATION_ENTITIES_NET_DETAILS_H
#define BGPSIMULATION_ENTITIES_NET_DETAILS_H

#include <IPv4Layer.h>

#include <memory>
#include <string>

#include "Link.h"

using namespace std;

class Link;  // forward declaration

/**
 * This abstracts the concept of a network card.
 * It can connect and disconnect to and from a \a Link.
 */
class NetworkCard {
   public:
    /**
     * Network interface.
     * @example \a eth0, \a wlan1, ...
     */
    string netInterface;

    /**
     * The IP associated with this network interface.
     */
    pcpp::IPv4Address IP;

    /**
     * The network mask associated with this network interface.
     */
    pcpp::IPv4Address netmask;

    /**
     * The link this network card is attached to, or \a nullptr when
     * disconnected.
     */
    shared_ptr<Link> link = nullptr;

    NetworkCard(string            netInterface,
                pcpp::IPv4Address IP,
                pcpp::IPv4Address netmask)
        : netInterface(std::move(netInterface)),
          IP(std::move(IP)),
          netmask(std::move(netmask)) {}

    /**
     * Connect the networkCard to the specified link.
     *
     * It does nothing if the networkCard is already connected to a link.
     *
     * @warning If the link is already fully associated with other network cards
     * the behavior is undefined.
     * @param linkToConnect The link to connect to.
     */
    void connect(const shared_ptr<Link>& linkToConnect);

    /**
     * Disconnect the networkCard from the specified link.
     *
     * It does nothing if the networkCard is not connected to the specified link
     * or no link at all.
     * @param linkToConnect The link to disconnect from.
     */
    void disconnect(const shared_ptr<Link>& linkToDisconnect);
};

#endif  // BGPSIMULATION_ENTITIES_NET_DETAILS_H
