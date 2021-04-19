#include "BGPUpdatePathAttribute.h"

#include <bitset>
#include <cstring>

size_t PathAttribute::pathAttributesToByteArray(
    const std::vector<PathAttribute>& vector,
    uint8_t*                          arrayToFill,
    size_t                            maximumSize) {
    size_t currentDataLen = 0;

    if (maximumSize == 0 || arrayToFill == nullptr) {
        return 0;
    }
    for (const auto& pathAttribute : vector) {
        if (maximumSize - currentDataLen < 1) {
            return 0;
        }
        auto attributeTypeFlags     = pathAttribute.attributeTypeFlags;
        arrayToFill[currentDataLen] = attributeTypeFlags;
        currentDataLen++;

        if (maximumSize - currentDataLen < 1) {
            return 0;
        }
        auto attributeTypeCode      = pathAttribute.attributeTypeCode;
        arrayToFill[currentDataLen] = attributeTypeCode;
        currentDataLen++;

        int lengthField_byteLength = 1;
        if (isFlagSet(attributeTypeFlags, EXTENDED)) {
            lengthField_byteLength = 2;
        }
        if (maximumSize - currentDataLen < lengthField_byteLength) {
            return 0;
        }
        auto lengthField_h = pathAttribute.getAttributeLength_h();
        auto lengthField_adjustedByteOrder = lengthField_byteLength == 1
                                                 ? (uint8_t)lengthField_h
                                                 : htobe16(lengthField_h);
        memcpy(arrayToFill + currentDataLen,
               &lengthField_adjustedByteOrder,
               lengthField_byteLength);
        currentDataLen += lengthField_byteLength;

        if (maximumSize - currentDataLen < lengthField_h) {
            return 0;
        }
        memcpy(arrayToFill + currentDataLen,
               pathAttribute.getAttributeValue_be(),
               lengthField_h);
        currentDataLen += lengthField_h;
    }

    return currentDataLen;
}

void PathAttribute::parsePathAttributes(
    uint8_t*                    byteArray,
    size_t                      arrayLen,
    std::vector<PathAttribute>& vectorToFill) {
    if (arrayLen == 0 || byteArray == nullptr) {
        return;
    }

    size_t byteCount = 0;

    while (byteCount < arrayLen) {
        PathAttribute newPathAttribute{};

        auto attributeTypeFlags =
            (AttributeTypeFlags_uint8_t)byteArray[byteCount];
        newPathAttribute.attributeTypeFlags = attributeTypeFlags;
        byteCount++;

        if (arrayLen - byteCount < 1) {
            vectorToFill.clear();
            return;
        }
        auto attributeTypeCode =
            (AttributeTypeCode_uint8_t)byteArray[byteCount];
        newPathAttribute.attributeTypeCode = attributeTypeCode;
        byteCount++;

        int lengthOfLengthField = 1;
        if (isFlagSet(attributeTypeFlags, EXTENDED)) {
            lengthOfLengthField = 2;
        }
        if (arrayLen - byteCount < lengthOfLengthField) {
            vectorToFill.clear();
            return;
        }
        uint16_t lengthField_h;
        if (lengthOfLengthField == 1) {
            lengthField_h = byteArray[byteCount];
            byteCount++;
        } else {
            uint16_t lengthField_be;
            memcpy(&lengthField_be, byteArray + byteCount, 2);
            lengthField_h = be16toh(lengthField_be);
            byteCount += 2;
        }
        // length is set below along with data

        if (arrayLen - byteCount < lengthField_h) {
            vectorToFill.clear();
            return;
        }

        newPathAttribute.setAttributeLengthAndValue(byteArray + byteCount,
                                                    lengthField_h);
        byteCount += lengthField_h;

        vectorToFill.push_back(newPathAttribute);
    }
}
std::string PathAttribute::toString() const {
    std::string output;

    output +=
        "- Flags: " + std::bitset<8>(attributeTypeFlags).to_string() + " \n";
    output +=
        "  Attribute TypeCode: " + std::to_string(attributeTypeCode) + " \n";
    output +=
        "  Attribute length: " + std::to_string(getAttributeLength_h()) + " \n";
    output += "  Attribute data:";
    for (int i = 0; i < getAttributeLength_h(); i++) {
        output += " " + std::bitset<8>(attributeData_be[i]).to_string();
    }
    output += "\n";

    return output;
}
