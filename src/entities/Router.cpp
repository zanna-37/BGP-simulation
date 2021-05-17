#include "Router.h"

Router::~Router() { delete bgpApplication; }

void Router::forwardMessage(stack<pcpp::Layer *> *layers,
                            NetworkCard *         networkCard) {
    networkCard->sendPacket(layers);
}

void Router::startInternal() {
    bgpApplication = new BGPApplication(this);
    bgpApplication->passiveOpen();
}