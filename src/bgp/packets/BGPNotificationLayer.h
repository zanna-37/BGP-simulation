#ifndef BGP_SIMULATION_BGP_PACKETS_BGPNOTIFICATIONLAYER_H
#define BGP_SIMULATION_BGP_PACKETS_BGPNOTIFICATIONLAYER_H

#include <cstddef>
#include <cstdint>
#include <string>

#include "BGPLayer.h"
#include "Layer.h"


class BGPNotificationLayer : public BGPLayer {
   public:
    enum ErrorCode_uint8_t : uint8_t {
        MSG_HEADER_ERR     = 1,
        OPEN_MSG_ERR       = 2,
        UPDATE_MSG_ERR     = 3,
        HOLD_TIMER_EXPIRED = 4,
        FSM_ERR            = 5,
        CEASE              = 6
    };

    enum ErrorSubcode_uint8_t : uint8_t {
        ERR_X_NO_SUB_ERR                 = 0,
        ERR_1_CONN_NOT_SYNC              = 1,
        ERR_1_BAD_MSG_LENGTH             = 2,
        ERR_1_BAD_MSG_TYPE               = 3,
        ERR_2_UNSUPPORTED_VERSION_NUM    = 1,
        ERR_2_BAD_PEER_AS                = 2,
        ERR_2_BAD_BGP_IDENTIFIER         = 3,
        ERR_2_UNSUPPORTED_OPTIONAL_PARAM = 4,
        // Subcode ERR_2_... = 5 is deprecated
        ERR_2_UNACCEPTABLE_HOLD_TIME      = 6,
        ERR_3_MALFORMED_ATTR_LIST         = 1,
        ERR_3_UNRECOGNIZED_WELLKNOWN_ATTR = 2,
        ERR_3_MISSING_WELLKNOWN_ATTR      = 3,
        ERR_3_ATTRIBUTE_FLAGS_ERR         = 4,
        ERR_3_ATTRIBUTE_LENGTH_ERR        = 5,
        ERR_3_INVALID_ORIGIN_ATTRIBUTE    = 6,
        // Subcode ERR_3_... = 7 is deprecated
        ERR_3_INVALID_NEXTHOP_ATTRIBUTE = 8,
        ERR_3_OPTIONAL_ATTR_ERR         = 9,
        ERR_3_INVALID_NETWORK_FIELD     = 10,
        ERR_3_MALFORMED_AS_PATH         = 11,
    };

#pragma pack(push, 1)
    struct BGPNotificationHeader : BGPCommonHeader {
        ErrorCode_uint8_t    errorCode;
        ErrorSubcode_uint8_t errorSubcode;
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
    BGPNotificationLayer(ErrorCode_uint8_t    errorCode,
                         ErrorSubcode_uint8_t errorSubcode);

    /**
     * A c'tor that creates a new BGP Notification message
     * @param[in] errorCode BGP notification error code
     * @param[in] errorSubCode BGP notification error sub code
     * @param[in] notificationData A byte array that contains the notification
     * data
     * @param[in] notificationDataLen The size of the byte array that contains
     * the notification data
     */
    BGPNotificationLayer(ErrorCode_uint8_t    errorCode,
                         ErrorSubcode_uint8_t errorSubcode,
                         const uint8_t*       notificationData,
                         size_t               notificationDataLen);

    size_t getNotificationDataLength() const;

    size_t getNotificationData_be(uint8_t* bufferToFill,
                                  size_t   maxLength) const;

    bool checkMessageErr(uint8_t*              subcode,
                         std::vector<uint8_t>* data_be8) const override;

   private:
    std::string toStringInternal() const override;
    void        computeCalculateFieldsInternal() const override;
    void        fillLayer(ErrorCode_uint8_t    errorCode,
                          ErrorSubcode_uint8_t errorSubcode,
                          const uint8_t*       notificationData,
                          size_t               notificationDataLen);
};


#endif  // BGP_SIMULATION_BGP_PACKETS_BGPNOTIFICATIONLAYER_H
