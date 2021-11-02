#include "NetUtils.h"

uint32_t NetUtils::prefixToNetmask(uint8_t prefixLength) {
    return ~(UINT32_MAX >> prefixLength);
}
