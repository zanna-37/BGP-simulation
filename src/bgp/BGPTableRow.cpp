#include "BGPTableRow.h"

BGPTableRow::BGPTableRow(pcpp::IPv4Address networkIP,
                         pcpp::IPv4Address networkMask,
                         pcpp::IPv4Address nextHop,
                         char              origin,
                         NetworkCard*      networkCard,
                         ASPath            asPath,
                         int               metric           = 0,
                         int               localPreferences = 0,
                         int               weight           = 0)
    : networkIP(networkIP),
      networkMask(networkMask),
      nextHop(nextHop),
      metric(metric),
      localPreferences(localPreferences),
      origin(origin),
      networkCard(networkCard),
      asPath(asPath) {}