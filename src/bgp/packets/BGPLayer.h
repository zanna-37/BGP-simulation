#ifndef BGP_SIMULATION_BGP_PACKETS_BGPLAYER_H
#define BGP_SIMULATION_BGP_PACKETS_BGPLAYER_H

#include <cstddef>
#include <cstdint>
#include <string>

#include "Layer.h"
#include "ProtocolType.h"


// TODO see https://tools.ietf.org/html/rfc4271#page-11

class BGPLayer : public pcpp::Layer {
   public:
#pragma pack(push, 1)
    struct BGPCommonHeader {
        uint8_t marker[16];
        /**
         * @warning This must be always kept in sync with the real length of the
         * layer and promptly updated when it changes. However, its value could
         * be wrong or invalid, for example in case we parse a received packet.
         */
        uint16_t length_be;
        uint8_t  type;
    };
#pragma pack(pop)

    enum BGPMessageType : uint8_t {
        OPEN         = 1,
        UPDATE       = 2,
        NOTIFICATION = 3,
        KEEPALIVE    = 4
    };


    static BGPLayer* parseBGPLayerOrNull(uint8_t*      data,
                                         size_t        dataLength,
                                         Layer*        prevLayer,
                                         pcpp::Packet* packet);

    void computeCalculateFields() override;

    /**
     * Does nothing for this layer (it is always last)
     */
    void parseNextLayer() override {}

    BGPCommonHeader* getCommonHeaderOrNull() const;

    /**
     * @brief Check the common header for errors
     *
     * @param header Message header received
     * @param subcode Subcode error for invalid header
     * @return true Header valid
     * @return false Header invalid
     */
    static bool checkMessageHeader(BGPLayer::BGPCommonHeader* header,
                                   uint8_t*                   subcode);

    /**
     * @brief Check the Marker of the common header to be all ones
     *
     * @param marker
     * @return true
     * @return false
     */
    static bool checkMarker(uint8_t marker[16]);

    size_t getHeaderLen() const override;

    static std::string getBGPMessageTypeName(BGPMessageType type);

    std::string toString() const override;

    pcpp::OsiModelLayer getOsiModelLayer() const override {
        return pcpp::OsiModelApplicationLayer;
    }

    virtual uint8_t getBGPMessageType() const = 0;

   protected:
    // protected c'tors, this class cannot be instanciated by users
    BGPLayer() { m_Protocol = pcpp::GenericPayload; }

    BGPLayer(uint8_t*      data,
             size_t        dataLen,
             Layer*        prevLayer,
             pcpp::Packet* packet)
        : Layer(data, dataLen, prevLayer, packet) {
        m_Protocol = pcpp::GenericPayload;
    }

    virtual std::string toStringInternal() const               = 0;
    virtual void        computeCalculateFieldsInternal() const = 0;

    /**
     * @brief Check if the specific message contains any error
     *
     * @param subcode Message specific subcode
     * @return true
     * @return false
     */
    virtual bool checkMessageErr(uint8_t subcode) const = 0;
};


#endif  // BGP_SIMULATION_BGP_PACKETS_BGPLAYER_H
