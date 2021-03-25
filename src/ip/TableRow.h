#ifndef TABLEROW_H
#define TABLEROW_H

#include <IPv4Layer.h>

#include <iomanip>
#include <iostream>

#include "../logger/Logger.h"

class TableRow {
   public:
    pcpp::IPv4Address networkIP;
    pcpp::IPv4Address defaultGateway;
    pcpp::IPv4Address netmask;
    std::string       netInterface;
    TableRow(pcpp::IPv4Address networkIP,
             pcpp::IPv4Address netmask,
             pcpp::IPv4Address defaultGateway,
             std::string       netInterface);

    ~TableRow(){};
};

#endif  // TABLEROW_H