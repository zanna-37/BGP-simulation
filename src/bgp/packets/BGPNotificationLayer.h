#ifndef BGP_SIMULATION_BGP_PACKETS_BGPNOTIFICATIONLAYER_H
#define BGP_SIMULATION_BGP_PACKETS_BGPNOTIFICATIONLAYER_H


#include "BGPLayer.h"

class BGPNotificationLayer : public BGPLayer {
   public:
#pragma pack(push, 1)
    struct BGPNotificationHeader : BGPCommonHeader {
        uint8_t errorCode;
        uint8_t errorSubcode;
    };
#pragma pack(pop)

    BGPNotificationHeader* getNotificationHeaderOrNull() const;

    uint8_t getBGPMessageType() const override {
        return BGPMessageType::NOTIFICATION;
    }

    /**
     * A constructor that creates the layer from an existing packet raw data
     * @param[in] data A pointer to the raw data
     * @param[in] dataLen Size of the data in bytes
     * @param[in] prevLayer A pointer to the previous layer
     * @param[in] packet A pointer to the Packet instance where layer will be
     * stored in
     */
    BGPNotificationLayer(uint8_t*      data,
                         size_t        dataLen,
                         Layer*        prevLayer,
                         pcpp::Packet* packet)
        : BGPLayer(data, dataLen, prevLayer, packet) {
        // Does nothing since everything is already managed by BGPLayer and
        // Layer parent c'tors
    }

    /**
     * A c'tor that creates a new BGP NOTIFICATION message
     * @param[in] errorCode BGP notification error code
     * @param[in] errorSubCode BGP notification error sub code
     */
    BGPNotificationLayer(uint8_t errorCode, uint8_t errorSubcode);

    /**
     * A c'tor that creates a new BGP Notification message
     * @param[in] errorCode BGP notification error code
     * @param[in] errorSubCode BGP notification error sub code
     * @param[in] notificationData A byte array that contains the notification
     * data
     * @param[in] notificationDataLen The size of the byte array that contains
     * the notification data
     */
    BGPNotificationLayer(uint8_t        errorCode,
                         uint8_t        errorSubcode,
                         const uint8_t* notificationData,
                         size_t         notificationDataLen);

    size_t getNotificationDataLength() const;

    size_t getNotificationData_be(uint8_t* bufferToFill,
                                  size_t   maxLength) const;

   private:
    std::string toStringInternal() const override;
    void        computeCalculateFieldsInternal() const override;
    void        fillLayer(uint8_t        errorCode,
                          uint8_t        errorSubcode,
                          const uint8_t* notificationData,
                          size_t         notificationDataLen);
};


#endif  // BGP_SIMULATION_BGP_PACKETS_BGPNOTIFICATIONLAYER_H
