#include "EndPoint.h"

#include "../logger/Logger.h"

void EndPoint::forwardMessage(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers,
    NetworkCard*                                              networkcard) {
    // drop packet
    L_ERROR(ID, "Endpoint can't handle message forwarding. Dropping packet");
}
