#ifndef BGP_SIMULATION_BGP_PACKETS_BGPOPENLAYER_H
#define BGP_SIMULATION_BGP_PACKETS_BGPOPENLAYER_H


#include "BGPLayer.h"
#include "IPv4Layer.h"

class BGPOpenLayer : public BGPLayer {
   public:
    struct BGPOpenHeader : BGPCommonHeader {
        uint8_t  version;
        uint16_t myAutonomousSystemNumber;
        uint16_t holdTime;
        uint32_t BGPIdentifier;
        uint8_t  optionalParametersLength;
    };

    BGPOpenHeader* getOpenHeader() const;

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

   private:
    std::string toStringInternal() const override;
    void        computeCalculateFieldsInternal() const override;
};


#endif  // BGP_SIMULATION_BGP_PACKETS_BGPOPENLAYER_H
