#include "BGPUpdatePathAttribute.h"

#include <endian.h>

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

void PathAttribute::asPathToAttributeDataArray_be(
    const uint8_t                asType,
    const uint8_t                asPathLen,
    const std::vector<uint16_t>& asPath,
    std::vector<uint8_t>&        asData_be) {
    if (asType == 1 || asType == 2) {
        asData_be.push_back(asType);
    } else {
        L_ERROR("ASDataBld", "AS Segment Type not handled");
    }
    if (asPathLen == asPath.size()) {
        asData_be.push_back(asPathLen);
    } else {
        L_ERROR(
            "ASDataBld",
            "AS path segment length not matching wiht the nuber of AS numbers");
    }

    for (auto AS_h : asPath) {
        uint16_t AS_be16 = htobe16(AS_h);
        asData_be.push_back((uint8_t)(AS_be16));
        asData_be.push_back((uint8_t)(AS_be16 >> 8));
    }
}

void PathAttribute::attributeDataArray_beToAsPath(
    const uint8_t*         asData_be,
    const size_t           asData_be_length,
    uint8_t&               asType,
    std::vector<uint16_t>& asPath) {
    auto currentByte = 0;
    if (currentByte < asData_be_length) {
        asType = asData_be[currentByte++];
        if (asType != 1 && asType != 2) {
            L_ERROR("ASDataBld", "AS Segment Type not handled");
        }

        if (currentByte < asData_be_length) {
            uint8_t declaredASPathLen = asData_be[currentByte++];

            uint8_t twoTimesTheRealASPathLen = asData_be_length - currentByte;
            if (twoTimesTheRealASPathLen % 2 == 0) {
                if (declaredASPathLen == (twoTimesTheRealASPathLen / 2)) {
                    while (currentByte < asData_be_length) {
                        uint8_t  low  = asData_be[currentByte++];
                        uint8_t  high = asData_be[currentByte++];
                        uint16_t tmp =
                            be16toh((uint16_t)low + ((uint16_t)high << 8));
                        asPath.push_back(tmp);
                    }
                } else {
                    L_FATAL("ASDataBld",
                            "The declared ASPathLen is not respected");
                }
            } else {
                L_FATAL("ASDataBld", "asData_be is malformed");
            }
        } else {
            L_FATAL("ASDataBld", "asData_be is too short");
        }
    } else {
        L_FATAL("ASDataBld", "asData_be is empty");
    }
}

bool PathAttribute::checkAsPathAttribute() const {
    // XXX: NOTE: the constructor and check of the AS segment needs to be
    // implemented for paths longer than 255

    size_t   len = this->getAttributeLength_h();
    uint8_t* val = this->getAttributeValue_be();

    if (len < 4) {
        L_ERROR("AsPathChk",
                "AS_PATH segment too short, it must contain at list a "
                "complete segment");
        return false;
    }

    if (len > 255 * 2 + 2) {
        L_ERROR("AsPathChk",
                "AS_PATH Attribute check does not handle multiple segments");
        return false;
    }

    if (val[0] != 1 && val[0] != 2) {
        L_ERROR("AsPathChk", "AS_PATH segment type not recognised");
        return false;
    }


    if (val[1] == 0 || val[1] != (len - 2) / 2) {
        L_ERROR("AsPathChk",
                "AS_PATH segment length and attribute length do not match!");
        return false;
    }

    return true;
}

void PathAttribute::getAttribute_be8(std::vector<uint8_t>* data_be8) {
    data_be8->push_back(this->attributeTypeFlags);
    data_be8->push_back(this->attributeTypeCode);
    uint16_t tmp_len = this->getAttributeLength_h();
    data_be8->push_back((uint8_t)tmp_len);
    data_be8->push_back((uint8_t)(tmp_len >> 8));
    uint8_t* tmp_val = this->getAttributeValue_be();
    for (int i = 0; i < tmp_len; i++) {
        data_be8->push_back(tmp_val[i]);
    }
}
