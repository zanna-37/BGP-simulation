#ifndef BGPTABLEROW_H
#define BGPTABLEROW_H

#include <IPLayer.h>

#include "../entities/NetworkCard.h"
#include "ASPath.h"

class BGPTableRow {
    bool              valid     = true;
    bool              preferred = true;
    pcpp::IPv4Address networkIP;
    pcpp::IPv4Address networkMask;
    pcpp::IPv4Address nextHop;
    int               metric;
    int               localPreferences;
    int               weight;
    char              origin;
    ASPath            asPath;


    NetworkCard* networkCard;

    BGPTableRow(pcpp::IPv4Address,
                pcpp::IPv4Address,
                pcpp::IPv4Address,
                char,
                NetworkCard*,
                ASPath,
                int,
                int,
                int);
    ~BGPTableRow();
};

#endif