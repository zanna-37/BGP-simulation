#include "BGPKeepaliveLayer.h"

#include <endian.h>

#include <cstring>


void BGPKeepaliveLayer::computeCalculateFieldsInternal() const {}

std::string BGPKeepaliveLayer::toStringInternal() const { return ""; }

BGPKeepaliveLayer::BGPKeepaliveLayer() : BGPLayer() {
    const size_t headerLen = sizeof(BGPCommonHeader);
    m_DataLen              = headerLen;
    m_Data                 = new uint8_t[headerLen];
    memset(m_Data, 0, headerLen);

    BGPCommonHeader* bgpCommonHeader =
        getCommonHeaderOrNull();  // bgpCommonHeader is not null as we just
                                  // populated m_Data and m_DataLen

    bgpCommonHeader->length_be = htobe16(headerLen);
}
