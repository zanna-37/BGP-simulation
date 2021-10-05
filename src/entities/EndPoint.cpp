#include "EndPoint.h"

#include <IcmpLayer.h>

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

void EndPoint::ping(pcpp::IPv4Address dstAddr) {
    L_ERROR(ID, "Sending ping message");

    auto icmpLayerToSend = std::make_unique<pcpp::IcmpLayer>();
    icmpLayerToSend->setEchoRequestData(0, 0, 0, nullptr, 0);

    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layersToSend =
        std::make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
    layersToSend->push(std::move(icmpLayerToSend));

    sendPacket(std::move(layersToSend), dstAddr);
}
