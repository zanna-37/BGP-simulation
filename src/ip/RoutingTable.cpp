#include "RoutingTable.h"

#include <iomanip>
#include <iostream>

RoutingTable::RoutingTable() { table = new std::vector<TableRow*>(); }
RoutingTable::~RoutingTable() {
    for (TableRow* row : *table) {
        delete row;
    }
    delete table;
}
void RoutingTable::insertRow(TableRow* row) { table->push_back(row); }

void RoutingTable::deleteRow(TableRow* row) {}

void RoutingTable::printTable() {
    printElement("Destination");
    printElement("Gateway");
    printElement("Genmask");
    printElement("Iface");
    std::cout << std::endl;
    for (TableRow* row : *table) {
        printElement(row->networkIP.toString());
        printElement(row->defaultGateway.toString());
        printElement(row->netmask.toString());
        printElement(row->netInterface);
        cout << std::endl;
    }
    std::cout << std::endl;
}

template <typename T>
void RoutingTable::printElement(T t) {
    const char separator = ' ';
    const int  width     = 16;
    std::cout << left << setw(width) << setfill(separator) << t;
}

NetworkCard* RoutingTable::findNextHop(pcpp::IPv4Address dstAddress) {
    int          longestMatch = -1;
    NetworkCard* result       = nullptr;
    for (TableRow* row : *table) {
        if (dstAddress.matchSubnet(row->networkIP, row->netmask) &&
            row->toCIDR() > longestMatch) {
            longestMatch = row->toCIDR();
            result       = row->networkCard;
        }
    }

    return result;
}