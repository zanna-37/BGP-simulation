#include "Router.h"

Router::~Router() { delete bgpApplication; }

void Router::forwardMessage(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers,
    NetworkCard*                                              networkCard) {
    networkCard->sendPacket(std::move(layers));
}

void Router::startInternal() {
    bgpApplication = new BGPApplication(this);
    bgpApplication->passiveOpen();
}