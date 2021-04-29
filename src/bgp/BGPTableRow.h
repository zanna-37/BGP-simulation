#ifndef BGPTABLEROW_H
#define BGPTABLEROW_H

#include <IPLayer.h>

#include <cstdint>
#include <cstdlib>

#include "../entities/NetworkCard.h"

class BGPTableRow {
   public:
    bool                  valid     = true;
    bool                  preferred = true;
    pcpp::IPv4Address     networkIP;
    pcpp::IPv4Address     networkMask;
    pcpp::IPv4Address     nextHop;
    uint32_t              metric;
    uint32_t              localPreferences;
    uint32_t              weight;
    char                  origin;
    std::vector<uint16_t> asPath;


    NetworkCard* networkCard;

    BGPTableRow(pcpp::IPv4Address,
                pcpp::IPv4Address,
                pcpp::IPv4Address,
                char,
                std::vector<uint16_t>,
                uint32_t,
                uint32_t,
                uint32_t);
    ~BGPTableRow(){};
};

#endif