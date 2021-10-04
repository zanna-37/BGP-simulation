#ifndef BGP_SIMULATION_ENTITIES_ENDPOINT_H
#define BGP_SIMULATION_ENTITIES_ENDPOINT_H

#include <memory>
#include <stack>
#include <string>
#include <utility>

#include "Device.h"
#include "IpAddress.h"
#include "Layer.h"
#include "NetworkCard.h"


using namespace std;

/**
 * An endpoint is a leaf node in a simulation. It can be a client or a server.
 */
class EndPoint : public virtual Device {
   public:
    EndPoint(std::string ID, pcpp::IPv4Address defaultGateway)
        : Device(std::move(ID), defaultGateway) {}

    ~EndPoint();

    /**
     * The endpoint drops a packet that does not have to process
     * @param layers the std::stack simulating the packet
     * @param networkCard the network card to forward. It should be null for the
     * enpoint case
     */
    void forwardMessage(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers,
        NetworkCard* networkCard) override;

    void ping(pcpp::IPv4Address destination);

    void bootUpInternal() override {}
};

#endif  // BGP_SIMULATION_ENTITIES_ENDPOINT_H
