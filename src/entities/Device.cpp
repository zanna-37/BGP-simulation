#include "Device.h"


Device::Device(string                 ID,
               pcpp::IPv4Address      defaultGateway,
               vector<NetworkCard *> *networkCards)
    : ID(std::move(ID)),
      defaultGateway(defaultGateway),
      networkCards(networkCards) {
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
}

NetworkCard *Device::getNetworkCardByInterfaceOrNull(
    const string &interfaceToSearch) {
    for (const auto &networkCard : *networkCards) {
        if (networkCard->netInterface == interfaceToSearch) {
            return networkCard;
        }
    }
    return nullptr;
}
