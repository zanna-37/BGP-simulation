#include "NetworkCard.h"

#include "../logger/Logger.h"
#include "Device.h"  // REFERENCE (forward declaration): https://pvigier.github.io/2018/02/09/dependency-graph.html

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
    if (link->connection_status == Connection_status::active) {
        L_DEBUG("Sending packet using " + netInterface + " through link");
        NetworkCard*   destination = link->getPeerNetworkCardOrNull(this);
        pcpp::EthLayer ethLayer(mac, destination->mac);
        layers->push(&ethLayer);
        pcpp::Packet packet;
        while (!layers->empty()) {
            packet.addLayer(layers->top());
            layers->pop();
        }
        packet.computeCalculateFields();
        link->sendPacket(&packet, destination);
    }
}

void NetworkCard::receivePacket(pcpp::Packet* receivedPacket) {
    // pcpp::EthLayer* ethHeader = packet->getLayerOfType<pcpp::EthLayer>();
    stack<pcpp::Layer*> layers;

    pcpp::Packet* packet = new pcpp::Packet(*receivedPacket);

    pcpp::Layer* currentLayer = packet->getLastLayer();
    while (currentLayer != nullptr) {
        layers.push(currentLayer);
        currentLayer = currentLayer->getPrevLayer();
    }
    // compute mac layer stuff

    layers.pop();
    owner->receivePacket(&layers, this);
    delete packet;
}
