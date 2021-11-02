#ifndef BGPSIMULATION_IP_TABLEROW_H
#define BGPSIMULATION_IP_TABLEROW_H

#include <string>

#include "IpAddress.h"

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

    ~TableRow() = default;

    /**
     * Transform the netmask in CIDR format
     * @return CIDR value of the netmask
     */
    [[nodiscard]] int toCIDR() const;
};

#endif  // BGPSIMULATION_IP_TABLEROW_H
