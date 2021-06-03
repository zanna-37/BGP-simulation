#include "../ip/IpManager.h"

#include <iomanip>

#include "../bgp/packets/BGPLayer.h"


void IpManager::buildRoutingTable(std::vector<TableRow> &     routingTable,
                                  std::vector<NetworkCard *> *networkCards,
                                  const pcpp::IPv4Address &   defaultGateway) {
    for (NetworkCard *networkCard : *networkCards) {
        pcpp::IPv4Address networkIP(networkCard->IP.toInt() &
                                    networkCard->netmask.toInt());

        routingTable.emplace_back(networkIP,
                                  pcpp::IPv4Address(networkCard->netmask),
                                  pcpp::IPv4Address::Zero,
                                  networkCard->netInterface,
                                  networkCard);

        // Set the default gateway
        if (defaultGateway.isValid() &&
            defaultGateway.matchSubnet(networkIP, networkCard->netmask)) {
            routingTable.emplace_back(pcpp::IPv4Address::Zero,
                                      pcpp::IPv4Address::Zero,
                                      pcpp::IPv4Address(defaultGateway),
                                      networkCard->netInterface,
                                      networkCard);
        }
    }
}

std::string IpManager::getTableCellAsString(const std::string &s) {
    const char         separator = ' ';
    const int          width     = 16;
    std::ostringstream oss;

    oss << std::left << std::setw(width) << std::setfill(separator) << s;
    return oss.str();
}

std::string IpManager::getRoutingTableAsString(
    const std::vector<TableRow> &routingTable) {
    std::string output;

    output += getTableCellAsString("Destination");
    output += getTableCellAsString("Gateway");
    output += getTableCellAsString("Genmask");
    output += getTableCellAsString("Iface");

    for (const TableRow &row : routingTable) {
        output += "\n";
        output += getTableCellAsString(row.networkIP.toString());
        output += getTableCellAsString(row.defaultGateway.toString());
        output += getTableCellAsString(row.netmask.toString());
        output += getTableCellAsString(row.netInterface);
    }

    return output;
}

NetworkCard *IpManager::findExitingNetworkCard(
    const pcpp::IPv4Address &dstAddress, const vector<TableRow> &routingTable) {
    int          longestMatch = -1;
    NetworkCard *result       = nullptr;
    for (const TableRow &row : routingTable) {
        if (dstAddress.matchSubnet(row.networkIP, row.netmask) &&
            row.toCIDR() > longestMatch) {
            longestMatch = row.toCIDR();
            result       = row.networkCard;
        }
    }

    return result;
}