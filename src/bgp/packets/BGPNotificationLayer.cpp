#include "BGPNotificationLayer.h"

#include <Logger.h>

#include <bitset>
#include <cstring>

BGPNotificationLayer::BGPNotificationHeader*
BGPNotificationLayer::getNotificationHeaderOrNull() const {
    if (m_DataLen >= sizeof(BGPNotificationHeader))
        return (BGPNotificationHeader*)m_Data;
    else {
        LOG_ERROR("The packet is to small to fit the BGP notification header");
        return nullptr;
    }
}

void BGPNotificationLayer::computeCalculateFieldsInternal() const {}

std::string BGPNotificationLayer::toStringInternal() const {
    BGPNotificationHeader* notificationHeader = getNotificationHeaderOrNull();
    if (notificationHeader) {
        std::string output;
        output +=
            "Error code: " + std::to_string(notificationHeader->errorCode) +
            "\n";
        output += "Error subcode: " +
                  std::to_string(notificationHeader->errorSubcode) + "\n";
        output += "Attribute data:";

        size_t notificationDatalength = getNotificationDataLength();
        if (notificationDatalength > 0) {
            auto* notificationData_be = new uint8_t[notificationDatalength];

            getNotificationData_be(notificationData_be, notificationDatalength);

            for (int i = 0; i < notificationDatalength; i++) {
                output +=
                    " " + std::bitset<8>(notificationData_be[i]).to_string();
            }
            delete[] notificationData_be;
        } else {
            output += "(empty)";
        }
        output += "\n";

        return output;
    } else {
        return "";
    }
}
BGPNotificationLayer::BGPNotificationLayer(ErrorCode_uint8_t    errorCode,
                                           ErrorSubcode_uint8_t errorSubcode) {
    fillLayer(errorCode, errorSubcode, nullptr, 0);
}

BGPNotificationLayer::BGPNotificationLayer(ErrorCode_uint8_t    errorCode,
                                           ErrorSubcode_uint8_t errorSubcode,
                                           const uint8_t* notificationData,
                                           size_t         notificationDataLen) {
    if (notificationData == nullptr || notificationDataLen < 0) {
        notificationDataLen = 0;
    }

    fillLayer(errorCode, errorSubcode, notificationData, notificationDataLen);
}

void BGPNotificationLayer::fillLayer(ErrorCode_uint8_t    errorCode,
                                     ErrorSubcode_uint8_t errorSubcode,
                                     const uint8_t*       notificationData,
                                     size_t               notificationDataLen) {
    // TODO place (and document) a max length for notificationDataLen
    const size_t headerLen =
        sizeof(BGPNotificationHeader) + notificationDataLen;
    m_DataLen = headerLen;
    m_Data    = new uint8_t[headerLen];
    memset(m_Data, 0, headerLen);

    BGPNotificationHeader* notificationHeader =
        getNotificationHeaderOrNull();  // notificationHeader is not null as we
                                        // just populated m_Data and m_DataLen

    notificationHeader->length_be    = htobe16(headerLen);
    notificationHeader->errorCode    = errorCode;
    notificationHeader->errorSubcode = errorSubcode;

    memcpy(m_Data + sizeof(BGPNotificationHeader),
           notificationData,
           notificationDataLen);
}

size_t BGPNotificationLayer::getNotificationDataLength() const {
    if (m_DataLen >= sizeof(BGPNotificationHeader)) {
        return m_DataLen - sizeof(BGPNotificationHeader);
    } else {
        return 0;
    }
}

size_t BGPNotificationLayer::getNotificationData_be(uint8_t* bufferToFill,
                                                    size_t   maxLength) const {
    size_t notificationDataLength = getNotificationDataLength();
    if (notificationDataLength > maxLength) {
        LOG_ERROR("The buffer is to small to fit the BGP notification data");
    } else {
        uint8_t* notificationData = m_Data + sizeof(BGPNotificationHeader);
        memcpy(bufferToFill, notificationData, notificationDataLength);
    }

    return notificationDataLength;
}
