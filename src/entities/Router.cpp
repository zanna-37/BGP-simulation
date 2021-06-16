#include "Router.h"

#include "../bgp/BGPApplication.h"
#include "Layer.h"
#include "NetworkCard.h"


Router::~Router() {
    // Note: all the subclasses need to call Device::shutdown() in their
    // deconstructor. Keep them in sync.
    shutdown();

    delete bgpApplication;
}

void Router::forwardMessage(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers,
    NetworkCard*                                              networkCard) {
    networkCard->sendPacket(std::move(layers));
}

void Router::bootUpInternal() {
    bgpApplication = new BGPApplication(this);
    bgpApplication->passiveOpenAll();
}
