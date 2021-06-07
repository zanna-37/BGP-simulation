#ifndef BGPSIMULATION_ENTITIES_ROUTER_H
#define BGPSIMULATION_ENTITIES_ROUTER_H

#include <string>

#include "../bgp/BGPApplication.h"
#include "Device.h"
#include "IPv4Layer.h"
#include "NetworkCard.h"

// forward declarations
#include "../bgp/BGPApplication.fwd.h"

using namespace std;

/**
 * A router is an internal node in a simulation which can handle packet routing.
 */
class Router : public virtual Device {
   public:
    /**
     * The Autonomous System number this router belongs to.
     */
    std::string AS_number;
    bool        running = false;

    BGPApplication *bgpApplication = nullptr;
    // TODO: announced_prefixes
    // TODO: local_preferences
    // TODO: trust
    std::vector<pcpp::IPv4Address> peer_addresses;

    Router(std::string                    ID,
           std::string                    AS_number,
           pcpp::IPv4Address              defaultGateway,
           std::vector<pcpp::IPv4Address> peer_addresses)
        : Device(std::move(ID), defaultGateway),
          AS_number(std::move(AS_number)) {
        this->peer_addresses.insert(this->peer_addresses.end(),
                                    peer_addresses.begin(),
                                    peer_addresses.end());
    }

    ~Router();

    /**
     * It forward the message to the next hop, if the message is not of this
     * device.
     * @param layers the std::stack simulation of the packet
     * @param networkCard the network card that will send the packet
     */
    void forwardMessage(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers,
        NetworkCard *networkCard) override;

    /**
     * Start the BGP application in passive open mode
     */
    void bootUpInternal() override;
};

#endif  // BGPSIMULATION_ENTITIES_ROUTER_H
