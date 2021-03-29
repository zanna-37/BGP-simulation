#ifndef BGPSIMULATION_ENTITIES_ROUTER_H
#define BGPSIMULATION_ENTITIES_ROUTER_H

#include <string>

#include "Device.h"
#include "NetworkCard.h"

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
    // TODO: announced_prefixes
    // TODO: local_preferences
    // TODO: trust

    Router(string ID, string AS_number, pcpp::IPv4Address defaultGateway)
        : Device(std::move(ID), defaultGateway),
          AS_number(std::move(AS_number)) {}

    ~Router() override = default;

    void forwardMessage(stack<pcpp::Layer *> *layers,
                        NetworkCard *         networkCard) override;
};

#endif  // BGPSIMULATION_ENTITIES_ROUTER_H
