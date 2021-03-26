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
                     networkCard->netInterface);

        routingTable.insertRow(row);

        if (defaultGateway.isValid() &&
            defaultGateway.matchSubnet(networkIP, networkCard->netmask)) {
            TableRow row(pcpp::IPv4Address::Zero,
                         pcpp::IPv4Address::Zero,
                         defaultGateway,
                         networkCard->netInterface);

            routingTable.insertRow(row);
        }
    }

    routingTable.printTable();

    deviceThread = new std::thread([&]() {
        while (running) {
            L_DEBUG(ID + "sleeping ...zzz...");
            this_thread::sleep_for(1s);
            for (NetworkCard *networkCard : *networkCards) {
                networkCard->sendPacket("Test");
            }
        }
    });
}
// do we need it? (just send it to the network card)
void Device::sendPacket(std::string data, NetworkCard networkCard) {
    networkCard.sendPacket(data);
}