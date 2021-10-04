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
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers =
        std::make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
    auto icmpLayer = std::make_unique<pcpp::IcmpLayer>();

    icmpLayer->setEchoRequestData(0, 0, 0, nullptr, 0);
    icmpLayer->computeCalculateFields();

    auto ipLayer = std::make_unique<pcpp::IPv4Layer>();
    ipLayer->setDstIPv4Address(dstAddr);
    NetworkCard* nextHopNetworkCard = getNextHopNetworkCardOrNull(dstAddr);

    if (nextHopNetworkCard == nullptr) {
        L_ERROR(ID, dstAddr.toString() + ": Destination unreachable");
    } else {
        L_DEBUG(ID, "Sending packet using " + nextHopNetworkCard->netInterface);
        ipLayer->setSrcIPv4Address(nextHopNetworkCard->IP);

        layers->push(std::move(icmpLayer));
        layers->push(std::move(ipLayer));
        nextHopNetworkCard->sendPacket(std::move(layers));
    }
}