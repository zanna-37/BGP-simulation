#include "NetworkCard.h"

#include "../logger/Logger.h"


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

void NetworkCard::sendPacket(std::string data) {
    NetworkCard* destination = link->getPeerNetworkCardOrNull(this);
    if (link->connection_status == Connection_status::active) {
        destination->receivePacket(data);
    }
}

void NetworkCard::receivePacket(std::string data) {
    L_DEBUG(netInterface + " received a packet: " + data);
}
