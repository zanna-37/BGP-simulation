#ifndef TABLEROW_H
#define TABLEROW_H

#include <IPv4Layer.h>

#include "../entities/NetworkCard.h"
#include "../logger/Logger.h"

// forward declarations
#include "../entities/NetworkCard.fwd.h"

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
    /**
     * Transform the netmask in CIDR format
     * @return CIDR value of the netmask
     */
    int toCIDR() const;
};

#endif  // TABLEROW_H