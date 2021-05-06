#ifndef BGP_SIMULATION_ENTITIES_ENDPOINT_H
#define BGP_SIMULATION_ENTITIES_ENDPOINT_H

#include <string>
#include <vector>

#include "Device.h"
#include "NetworkCard.h"

using namespace std;

/**
 * An endpoint is a leaf node in a simulation. It can be a client or a server.
 */
class EndPoint : public virtual Device {
   public:
    EndPoint(string ID, pcpp::IPv4Address defaultGateway)
        : Device(std::move(ID), defaultGateway) {}

    ~EndPoint() {}

    /**
     * The endpoint drops a packet that does not have to process
     * @param layers the std::stack simulating the packet
     * @param networkCard the network card to forward. It should be null for the
     * enpoint case
     */
    void forwardMessage(stack<pcpp::Layer *> *layers,
                        NetworkCard *         networkCard) override;

    void startInternal() {}
};

#endif  // BGP_SIMULATION_ENTITIES_ENDPOINT_H
