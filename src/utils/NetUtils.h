#ifndef BGP_SIMULATION_NETUTILS_H
#define BGP_SIMULATION_NETUTILS_H

#include <cstdint>

class NetUtils {
   public:
    static uint32_t prefixToNetmask(uint8_t prefixLen);
};


#endif  // BGP_SIMULATION_NETUTILS_H
