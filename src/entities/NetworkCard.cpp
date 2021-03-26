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

void NetworkCard::sendPacket(pcpp::Packet* packet) {
    if (link->connection_status == Connection_status::active) {
        NetworkCard*   destination = link->getPeerNetworkCardOrNull(this);
        pcpp::EthLayer ethHeader(mac, destination->mac);
        packet->addLayer(&ethHeader);
        link->sendPacket(packet, destination);
    }
}

void NetworkCard::receivePacket(pcpp::Packet* packet) {
    pcpp::EthLayer* ethHeader = packet->getLayerOfType<pcpp::EthLayer>();

    // compute mac layer stuff

    packet->removeLayer(ethHeader->getProtocol());
    owner->receivePacket(packet);
}
