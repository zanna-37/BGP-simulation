#include "EndPoint.h"

#include "../logger/Logger.h"

EndPoint::~EndPoint() {
    // Note: all the subclasses need to call Device::shutdown() in their
    // deconstructor. Keep them in sync.
    shutdown();
}

void EndPoint::forwardMessage(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers,
    NetworkCard*                                              networkcard) {
    // drop packet
    L_ERROR(ID, "Endpoint can't handle message forwarding. Dropping packet");
}
