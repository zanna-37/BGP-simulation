#include "Router.h"

void Router::forwardMessage(stack<pcpp::Layer *> *layers,
                            NetworkCard *         networkCard) {
    networkCard->sendPacket(layers);
}

void Router::startInternal() {

    bgpApplication = new BGPApplication(this);
    bgpApplication->passiveOpen();
}