#include "TableRow.h"

TableRow::TableRow(pcpp::IPv4Address networkIP,
                   pcpp::IPv4Address netmask,
                   pcpp::IPv4Address defaultGateway,
                   std::string       netInterface)
    : networkIP(networkIP),
      netmask(netmask),
      defaultGateway(defaultGateway),
      netInterface(netInterface) {}
