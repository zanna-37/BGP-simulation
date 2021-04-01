#ifndef TABLEROW_H
#define TABLEROW_H

#include <IPv4Layer.h>

#include <iomanip>
#include <iostream>

#include "../entities/NetworkCard.h"
#include "../logger/Logger.h"

class TableRow {
   public:
    pcpp::IPv4Address networkIP;
    pcpp::IPv4Address defaultGateway;
    pcpp::IPv4Address netmask;
    std::string       netInterface;
    NetworkCard*      networkCard = nullptr;
    TableRow(pcpp::IPv4Address networkIP,
             pcpp::IPv4Address netmask,
             pcpp::IPv4Address defaultGateway,
             std::string       netInterface,
             NetworkCard*      networkCard);
    ~TableRow(){};

    // TableRow operator=(const TableRow& other);

    int toCIDR();
};

#endif  // TABLEROW_H