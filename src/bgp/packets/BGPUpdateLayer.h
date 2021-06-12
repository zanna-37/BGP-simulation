#ifndef BGP_SIMULATION_BGP_PACKETS_BGPUPDATELAYER_H
#define BGP_SIMULATION_BGP_PACKETS_BGPUPDATELAYER_H

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "BGPLayer.h"
#include "BGPUpdateLengthAndIpPrefix.h"
#include "BGPUpdatePathAttribute.h"
#include "Layer.h"


class BGPUpdateLayer : public BGPLayer {
   public:
    uint8_t getBGPMessageType() const override {
        return BGPMessageType::UPDATE;
    }

    /**
     * A constructor that creates the layer from an existing packet raw data
     * @param[in] data A pointer to the raw data
     * @param[in] dataLen Size of the data in bytes
     * @param[in] prevLayer A pointer to the previous layer
     * @param[in] packet A pointer to the Packet instance where layer will be
     * stored in
     */
    BGPUpdateLayer(uint8_t*      data,
                   size_t        dataLen,
                   Layer*        prevLayer,
                   pcpp::Packet* packet)
        : BGPLayer(data, dataLen, prevLayer, packet) {
        // Does nothing since everything is already managed by BGPLayer and
        // Layer parent c'tors
    }

    /**
     * A c'tor that creates a new BGP UPDATE message
     * @param[in] withdrawnRoutes A vector of withdrawn routes data. If left
     * empty (which is the default value) no withdrawn route information will be
     * written to the message
     * @param[in] pathAttributes A vector of path attributes data. If left empty
     * (which is the default value) no path attribute information will be
     * written to the message
     * @param[in] nlri A vector of network layer reachability data. If left
     * empty (which is the default value) no reachability information will be
     * written to the message
     */
    BGPUpdateLayer(const std::vector<LengthAndIpPrefix>& withdrawnRoutes =
                       std::vector<LengthAndIpPrefix>(),
                   const std::vector<PathAttribute>& pathAttributes =
                       std::vector<PathAttribute>(),
                   const std::vector<LengthAndIpPrefix>& nlri =
                       std::vector<LengthAndIpPrefix>());

    size_t getWithdrawnRoutesBytesLength() const;

    void getWithdrawnRoutes(
        std::vector<LengthAndIpPrefix>& withdrawnRoutes) const;

    size_t getPathAttributesBytesLength() const;

    void getPathAttributes(std::vector<PathAttribute>& pathAttributes) const;

    size_t getNetworkLayerReachabilityInfoBytesLength() const;

    void getNetworkLayerReachabilityInfo(
        std::vector<LengthAndIpPrefix>& nlri) const;

   private:
    std::string toStringInternal() const override;
    void        computeCalculateFieldsInternal() const override;
};


#endif  // BGP_SIMULATION_BGP_PACKETS_BGPUPDATELAYER_H
