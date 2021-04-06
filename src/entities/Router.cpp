#include "Router.h"
// FIXME
void Router::forwardMessage(stack<pcpp::Layer *> *layers,
                            NetworkCard *         networkCard) {
    networkCard->sendPacket(layers);
}