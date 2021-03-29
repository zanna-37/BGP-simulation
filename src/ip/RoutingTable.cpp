#include "RoutingTable.h"

void RoutingTable::insertRow(TableRow row) { table.push_back(row); }

void RoutingTable::deleteRow(TableRow row) {}

void RoutingTable::printTable() {
    printElement("Destination");
    printElement("Gateway");
    printElement("Genmask");
    printElement("Iface");
    std::cout << endl;
    for (TableRow row : table) {
        printElement(row.networkIP.toString());
        printElement(row.defaultGateway.toString());
        printElement(row.netmask.toString());
        printElement(row.netInterface);
        cout << endl;
    }
    std::cout << endl;
}

template <typename T>
void RoutingTable::printElement(T t) {
    const char separator = ' ';
    const int  width     = 16;
    std::cout << left << setw(width) << setfill(separator) << t;
}

TableRow* RoutingTable::findNextHop(pcpp::IPv4Address dstAddress) {
    int       longestMatch = -1;
    TableRow* result       = nullptr;
    for (TableRow row : table) {
        if (dstAddress.matchSubnet(row.networkIP, row.netmask) &&
            row.toCIDR() > longestMatch) {
            longestMatch = row.toCIDR();
            result       = &row;
        }
    }

    return result;
}