#ifndef BGP_SIMULATION_BGP_PACKETS_BGPKEEPALIVELAYER_H
#define BGP_SIMULATION_BGP_PACKETS_BGPKEEPALIVELAYER_H

#include <cstddef>
#include <cstdint>
#include <string>

#include "BGPLayer.h"
#include "Layer.h"


class BGPKeepaliveLayer : public BGPLayer {
   public:
    uint8_t getBGPMessageType() const override {
        return BGPMessageType::KEEPALIVE;
    }

    /**
     * A constructor that creates the layer from an existing packet raw data
     * @param[in] data A pointer to the raw data
     * @param[in] dataLen Size of the data in bytes
     * @param[in] prevLayer A pointer to the previous layer
     * @param[in] packet A pointer to the Packet instance where layer will be
     * stored in
     */
    BGPKeepaliveLayer(uint8_t*      data,
                      size_t        dataLen,
                      Layer*        prevLayer,
                      pcpp::Packet* packet)
        : BGPLayer(data, dataLen, prevLayer, packet) {
        // Does nothing since everything is already managed by BGPLayer and
        // Layer parent c'tors
    }

    /**
     * A c'tor that creates a new BGP KEEPALIVE message
     */
    BGPKeepaliveLayer();

    bool checkMessageErr(uint8_t subcode) const override;

   private:
    std::string toStringInternal() const override;
    void        computeCalculateFieldsInternal() const override;
};


#endif  // BGP_SIMULATION_BGP_PACKETS_BGPKEEPALIVELAYER_H
