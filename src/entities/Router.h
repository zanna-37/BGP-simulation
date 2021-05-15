#ifndef BGPSIMULATION_ENTITIES_ROUTER_H
#define BGPSIMULATION_ENTITIES_ROUTER_H

#include <IPv4Layer.h>

#include <string>

#include "../bgp/BGPApplication.h"
#include "Device.h"
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
    string AS_number;
    bool   running = false;

    std::vector<pcpp::IPv4Address> peerList;

    BGPApplication *bgpApplication = nullptr;
    // TODO: announced_prefixes
    // TODO: local_preferences
    // TODO: trust

    Router(string ID, string AS_number, pcpp::IPv4Address defaultGateway)
        : Device(std::move(ID), defaultGateway),
          AS_number(std::move(AS_number)) /*TODO add peer list*/ {}

    ~Router();

    /**
     * It forward the message to the next hop, if the message is not of this
     * device.
     * @param layers the std::stack simulation of the packet
     * @param networkCard the network card that will send the packet
     */
    void forwardMessage(stack<pcpp::Layer *> *layers,
                        NetworkCard *         networkCard) override;

    /**
     * Start the BGP application in passive open mode
     */
    void startInternal() override;
};

#endif  // BGPSIMULATION_ENTITIES_ROUTER_H
