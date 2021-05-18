#include "NetworkCard.h"

#include "../logger/Logger.h"

NetworkCard::NetworkCard(string            netInterface,
                         pcpp::IPv4Address IP,
                         pcpp::IPv4Address netmask,
                         pcpp::MacAddress  mac,
                         Device*           owner)
    : netInterface(std::move(netInterface)),
      IP(IP),
      netmask(netmask),
      owner(owner) {
    if (mac != pcpp::MacAddress::Zero) {
        this->mac = mac;
    } else {
        macGenerator(this->mac);
    }
}

NetworkCard::~NetworkCard() = default;

void NetworkCard::connect(const shared_ptr<Link>& linkToConnect) {
    if (link == nullptr) {
        linkToConnect->connect(this);
        link = linkToConnect;
    } else {
        L_ERROR(owner->ID,
                "(" + netInterface + ") This interface is already connected");
    }
}

void NetworkCard::disconnect(const shared_ptr<Link>& linkToDisconnect) {
    if (link.get() == linkToDisconnect.get()) {
        linkToDisconnect->disconnect(this);
        link = nullptr;
    } else if (link == nullptr) {
        L_ERROR(owner->ID,
                "(" + netInterface + ") This interface has no link connected");
    } else {
        L_ERROR(
            owner->ID,
            "(" + netInterface +
                ") This interface is not connected with the link specified");
    }
}

void NetworkCard::sendPacket(stack<pcpp::Layer*>* layers) {
    if (link->connection_status == ACTIVE) {
        L_DEBUG(owner->ID,
                "Sending packet using " + netInterface + " through link");
        NetworkCard* destination = link->getPeerNetworkCardOrNull(this);
        // TODO check destination is not null
        auto* ethLayer = new pcpp::EthLayer(mac, destination->mac);
        layers->push(ethLayer);
        auto* packet = new pcpp::Packet(100);
        while (!layers->empty()) {
            packet->addLayer(layers->top(), true);
            layers->pop();
        }
        packet->computeCalculateFields();
        link->sendPacket(packet, destination);
        delete packet;
    }
}

void NetworkCard::receivePacket(unique_ptr<pcpp::Packet>& receivedPacket) {
    L_DEBUG(owner->ID, "Enqueueing packet in " + netInterface + " queue");
    receivedPacketsQueue.push(std::move(receivedPacket));

    auto* event = new ReceivedPacketEvent(
        this, ReceivedPacketEvent::Description::PACKET_ARRIVED);
    owner->enqueueEvent(event);
}

void NetworkCard::handleNextPacket() {
    unique_ptr<pcpp::Packet> receivedPacket =
        std::move(receivedPacketsQueue.front());
    receivedPacketsQueue.pop();
    auto* layers = new stack<pcpp::Layer*>();

    pcpp::Layer* currentLayer = receivedPacket->getLastLayer();
    while (currentLayer != nullptr) {
        pcpp::ProtocolType protocol = currentLayer->getProtocol();
        layers->push(receivedPacket->detachLayer(protocol));
        currentLayer = receivedPacket->getLastLayer();
    }

    auto* ethLayer = dynamic_cast<pcpp::EthLayer*>(layers->top());
    layers->pop();
    // TODO compute mac layer stuff
    delete ethLayer;
    owner->receivePacket(layers, this);

    // FIXME
    L_DEBUG(owner->ID, std::to_string(layers->empty()));
    while (!layers->empty()) {
        pcpp::Layer* layer = layers->top();
        layers->pop();
        delete layer;
    }
    delete layers;
}