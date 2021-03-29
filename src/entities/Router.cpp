#include "Router.h"

void Router::forwardMessage(stack<pcpp::Layer *> *layers,
                            NetworkCard *         networkCard) {
    sendPacket(layers, networkCard);
}