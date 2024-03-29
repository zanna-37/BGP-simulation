#ifndef BGP_SIMULATION_BGP_PACKETS_BGPOPENLAYER_H
#define BGP_SIMULATION_BGP_PACKETS_BGPOPENLAYER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "BGPLayer.h"
#include "IpAddress.h"
#include "Layer.h"


class BGPOpenLayer : public BGPLayer {
   public:
#pragma pack(push, 1)
    struct BGPOpenHeader : BGPCommonHeader {
        uint8_t  version;
        uint16_t myAutonomousSystemNumber_be;
        uint16_t holdTime_be;
        uint32_t BGPIdentifier_be;
        /**
         * @warning This must be always kept in sync with the real length of the
         * layer and promptly updated when it changes. However, its value could
         * be wrong or invalid, for example in case we parse a received packet.
         */
        uint8_t optionalParametersLength;
    };
#pragma pack(pop)

    static const uint8_t version = 4;

    BGPOpenHeader* getOpenHeaderOrNull() const;

    uint8_t getBGPMessageType() const override { return BGPMessageType::OPEN; }

    /**
     * A constructor that creates the layer from an existing packet raw data
     * @param[in] data A pointer to the raw data
     * @param[in] dataLen Size of the data in bytes
     * @param[in] prevLayer A pointer to the previous layer
     * @param[in] packet A pointer to the Packet instance where layer will be
     * stored in
     */
    BGPOpenLayer(uint8_t*      data,
                 size_t        dataLen,
                 Layer*        prevLayer,
                 pcpp::Packet* packet)
        : BGPLayer(data, dataLen, prevLayer, packet) {
        // Does nothing since everything is already managed by BGPLayer and
        // Layer parent c'tors
    }

    /**
     * A c'tor that creates a new BGP OPEN message
     * @param[in] myAutonomousSystemNumber The Autonomous System number of the
     * sender
     * @param[in] holdTime The number of seconds the sender proposes for the
     * value of the Hold Timer
     * @param[in] BGPIdentifier The BGP Identifier of the sender
     * @param[in] optionalParams A vector of optional parameters. This parameter
     * is optional and if not provided no parameters will be set on the message
     */
    BGPOpenLayer(uint16_t                 myAutonomousSystemNumber,
                 uint16_t                 holdTime,
                 const pcpp::IPv4Address& BGPIdentifier);

    bool checkMessageErr(uint8_t*              subcode,
                         std::vector<uint8_t>* data_be8) const override;

    /**
     * @brief The check of the AS number is out of the scope of the RFC 4271
     *
     * @return true Valid AS
     * @return false Invalid AS
     */
    bool checkAS() const { return true; }
    // TODO We may need to implement the AS checking but I don't think so

    /**
     * @brief Check that the address is not private and not multicast
     *
     * @return true
     * @return false
     */
    bool checkValidIP() const;

   private:
    std::string toStringInternal() const override;
    void        computeCalculateFieldsInternal() const override;
};


#endif  // BGP_SIMULATION_BGP_PACKETS_BGPOPENLAYER_H
