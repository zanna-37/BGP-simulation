#ifndef BGP_SIMULATION_BGP_PACKETS_BGPLAYER_H
#define BGP_SIMULATION_BGP_PACKETS_BGPLAYER_H

#include <Layer.h>

// TODO see https://tools.ietf.org/html/rfc4271#page-11

class BGPLayer : public pcpp::Layer {
   public:
    struct BgpCommonHeader {
        uint8_t  marker[16];
        uint16_t length;
        uint8_t  type;
    };

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

    BgpCommonHeader* getCommonHeader() const;

    size_t getHeaderLen() const override;

    static std::string getBGPMessageTypeName(BGPMessageType type);

    std::string toString() const override;

    pcpp::OsiModelLayer getOsiModelLayer() const override {
        return pcpp::OsiModelApplicationLayer;
    }

    virtual uint8_t getBGPMessageType() const = 0;

   protected:
    // protected c'tors, this class cannot be instanciated by users
    BGPLayer() = default;

    BGPLayer(uint8_t*      data,
             size_t        dataLen,
             Layer*        prevLayer,
             pcpp::Packet* packet)
        : Layer(data, dataLen, prevLayer, packet) {
        m_Protocol = pcpp::GenericPayload;
    }

    virtual std::string toStringInternal() const = 0;
};


#endif  // BGP_SIMULATION_BGP_PACKETS_BGPLAYER_H
