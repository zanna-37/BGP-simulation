#include "EndPoint.h"

#include "../logger/Logger.h"

void EndPoint::forwardMessage(stack<pcpp::Layer *> *layers,
                              NetworkCard *         networkcard) {
    // drop packet
    L_ERROR(ID, "Endpoint can't handle message forwarding. Dropping packet");
}
