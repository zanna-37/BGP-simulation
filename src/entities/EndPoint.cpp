#include "EndPoint.h"

#include "../logger/Logger.h"

void EndPoint::forwardMessage(stack<pcpp::Layer *> *layers,
                              NetworkCard *         networkcard) {
    // drop packet
    L_INFO("Endoint can't handle message forwarding. Dropping packet");
}
