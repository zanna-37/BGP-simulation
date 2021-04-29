#include "BGPTableRow.h"

BGPTableRow::BGPTableRow(pcpp::IPv4Address     networkIP,
                         pcpp::IPv4Address     networkMask,
                         pcpp::IPv4Address     nextHop,
                         char                  origin,
                         std::vector<uint16_t> asPath,
                         uint32_t              metric           = 0,
                         uint32_t              localPreferences = 0,
                         uint32_t              weight           = 0)
    : networkIP(networkIP),
      networkMask(networkMask),
      nextHop(nextHop),
      metric(metric),
      localPreferences(localPreferences),
      weight(weight),
      origin(origin),
      asPath(asPath) {}