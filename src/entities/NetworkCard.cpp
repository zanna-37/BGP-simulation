#include "NetworkCard.h"

#include "../logger/Logger.h"

NetworkCard::NetworkCard(string            netInterface,
                         pcpp::IPv4Address IP,
                         pcpp::IPv4Address netmask,
                         pcpp::MacAddress  mac,
                         Device*           owner)
    : netInterface(std::move(netInterface)),
      IP(std::move(IP)),
      netmask(std::move(netmask)),
      owner(owner) {
    if (mac != pcpp::MacAddress::Zero) {
        this->mac = mac;
    } else {
        macGenerator(this->mac);
    }
}

NetworkCard::~NetworkCard() {}

void NetworkCard::connect(const shared_ptr<Link>& linkToConnect) {
    if (link == nullptr) {
        linkToConnect->connect(this);
        link = linkToConnect;
    } else {
        L_ERROR("(" + netInterface + ") This interface is already connected");
    }
}

void NetworkCard::disconnect(const shared_ptr<Link>& linkToDisconnect) {
    if (link.get() == linkToDisconnect.get()) {
        linkToDisconnect->disconnect(this);
        link = nullptr;
    } else if (link == nullptr) {
        L_ERROR("(" + netInterface + ") This interface has no link connected");
    } else {
        L_ERROR("(" + netInterface +
                ") This interface is not connected with the link specified");
    }
}

void NetworkCard::sendPacket(stack<pcpp::Layer*>* layers) {
    if (link->connection_status == active) {
        L_DEBUG("Sending packet using " + netInterface + " through link");
        NetworkCard*    destination = link->getPeerNetworkCardOrNull(this);
        pcpp::EthLayer* ethLayer    = new pcpp::EthLayer(mac, destination->mac);
        layers->push(ethLayer);
        pcpp::Packet* packet = new pcpp::Packet(100);
        while (!layers->empty()) {
            packet->addLayer(layers->top(), true);
            layers->pop();
        }
        delete layers;
        packet->computeCalculateFields();
        link->sendPacket(packet, destination);
    }
}

void NetworkCard::receivePacket(pcpp::Packet* receivedPacket) {
    L_DEBUG("Enqueueing event in " + netInterface + " queue");
    receivedPacketsQueue.push(receivedPacket);
    ReceivedPacketEvent* event = new ReceivedPacketEvent(
        this, ReceivedPacketEvent::Description::PACKET_ARRIVED);
    owner->enqueueEvent(event);
}

void NetworkCard::handleNextPacket() {
    pcpp::Packet*        receivedPacket = receivedPacketsQueue.front();
    stack<pcpp::Layer*>* layers         = new stack<pcpp::Layer*>();

    pcpp::Layer* currentLayer = receivedPacket->getLastLayer();
    while (currentLayer != nullptr) {
        pcpp::ProtocolType protocol = currentLayer->getProtocol();
        layers->push(receivedPacket->detachLayer(protocol));
        currentLayer = receivedPacket->getLastLayer();
    }


    delete receivedPacket;


    pcpp::EthLayer* ethLayer = dynamic_cast<pcpp::EthLayer*>(layers->top());
    layers->pop();
    // compute mac layer stuff
    delete ethLayer;
    owner->receivePacket(layers, this);
}