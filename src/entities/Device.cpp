#include "Device.h"


Device::Device(string ID, pcpp::IPv4Address defaultGateway)
    : ID(std::move(ID)), defaultGateway(defaultGateway) {}

NetworkCard *Device::getNetworkCardByInterfaceOrNull(

    const string &interfaceToSearch) {
    for (const auto &networkCard : *networkCards) {
        if (networkCard->netInterface == interfaceToSearch) {
            return networkCard;
        }
    }
    return nullptr;
}

void Device::addCards(vector<NetworkCard *> *networkCards) {
    this->networkCards = networkCards;
}


void Device::start() {
    for (NetworkCard *networkCard : *networkCards) {
        pcpp::IPv4Address networkIP(
            (networkCard->IP.toInt() & networkCard->netmask.toInt()));

        TableRow row(networkIP,
                     networkCard->netmask,
                     pcpp::IPv4Address::Zero,
                     networkCard->netInterface,
                     networkCard);

        routingTable.insertRow(row);

        if (defaultGateway.isValid() &&
            defaultGateway.matchSubnet(networkIP, networkCard->netmask)) {
            TableRow row(pcpp::IPv4Address::Zero,
                         pcpp::IPv4Address::Zero,
                         defaultGateway,
                         networkCard->netInterface,
                         networkCard);

            routingTable.insertRow(row);
        }
    }

    routingTable.printTable();

    // deviceThread = new std::thread([&]() {
    //     while (running) {
    //         L_DEBUG(ID + "sleeping ...zzz...");
    //         this_thread::sleep_for(1s);
    //         // for (NetworkCard *networkCard : *networkCards) {
    //         //     // networkCard->sendPacket("Test");
    //         // }
    //     }
    // });
}

void Device::sendPacket(stack<pcpp::Layer *> *layers,
                        NetworkCard *         networkCard) {
    L_DEBUG("Sending packet from " + ID + " using " +
            networkCard->netInterface);
    networkCard->sendPacket(layers);
}

void Device::receivePacket(stack<pcpp::Layer *> *layers, NetworkCard *origin) {
    pcpp::IPv4Layer * ipLayer = dynamic_cast<pcpp::IPv4Layer *>(layers->top());
    pcpp::IPv4Address dstAddress = ipLayer->getDstIPv4Address();

    if (dstAddress == origin->IP) {
        L_DEBUG("processing message");
        processMessage(layers);
    } else {
        TableRow *nextHop_row = routingTable.findNextHop(dstAddress);
        if (nextHop_row == nullptr) {
            L_ERROR("DESTINATION UNREACHABLE");
        } else {
            forwardMessage(layers, nextHop_row->networkCard);
        }
    }
}

void Device::processMessage(stack<pcpp::Layer *> *layers) {
    L_DEBUG("RICEVUTO: " + ID);
}