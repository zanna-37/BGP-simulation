#include "TableRow.h"

TableRow::TableRow(pcpp::IPv4Address networkIP,
                   pcpp::IPv4Address netmask,
                   pcpp::IPv4Address defaultGateway,
                   std::string       netInterface,
                   NetworkCard*      networkCard)
    : networkIP(networkIP),
      netmask(netmask),
      defaultGateway(defaultGateway),
      netInterface(netInterface),
      networkCard(networkCard) {}


int TableRow::toCIDR() {
    int cidr = 0;

    const uint8_t* netmask_bytes = netmask.toBytes();

    for (int i = 0; i < 4; i++) {
        switch (netmask_bytes[i]) {
            case 0x80:
                cidr += 1;
                break;

            case 0xC0:
                cidr += 2;
                break;

            case 0xE0:
                cidr += 3;
                break;

            case 0xF0:
                cidr += 4;
                break;

            case 0xF8:
                cidr += 5;
                break;

            case 0xFC:
                cidr += 6;
                break;

            case 0xFE:
                cidr += 7;
                break;

            case 0xFF:
                cidr += 8;
                break;

            default:
                return cidr;
                break;
        }
    }
    return cidr;
}
