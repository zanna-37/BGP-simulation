#include "BGPUpdateLengthAndIpPrefix.h"

#include <bitset>

size_t LengthAndIpPrefix::lengthAndIpPrefixDataToByteArray(
    const std::vector<LengthAndIpPrefix>& vector,
    uint8_t*                              arrayToFill,
    size_t                                maximumSize) {
    size_t currentDataLen = 0;

    if (maximumSize == 0 || arrayToFill == nullptr) {
        return 0;
    }
    for (auto lengthAndIpPrefix : vector) {
        if (maximumSize - currentDataLen < 1) {
            return 0;
        }
        auto prefixLength           = lengthAndIpPrefix.prefixLength;
        arrayToFill[currentDataLen] = prefixLength;
        currentDataLen++;

        // currentDataLen is updated in the if-chain below

        const uint8_t* octets = lengthAndIpPrefix.ipPrefix.toBytes();

        if (prefixLength > 0) {
            if (maximumSize - currentDataLen < 1) {
                return 0;
            }
            arrayToFill[currentDataLen] = octets[0];
            currentDataLen++;
        }
        if (prefixLength > 8) {
            if (maximumSize - currentDataLen < 1) {
                return 0;
            }
            arrayToFill[currentDataLen] = octets[1];
            currentDataLen++;
        }
        if (prefixLength > 16) {
            if (maximumSize - currentDataLen < 1) {
                return 0;
            }
            arrayToFill[currentDataLen] = octets[2];
            currentDataLen++;
        }
        if (prefixLength > 24 && prefixLength <= 32) {
            if (maximumSize - currentDataLen < 1) {
                return 0;
            }
            arrayToFill[currentDataLen] = octets[3];
            currentDataLen++;
        }
    }

    return currentDataLen;
}

void LengthAndIpPrefix::parsePrefixAndIPData(
    uint8_t*                        byteArray,
    size_t                          arrayLen,
    std::vector<LengthAndIpPrefix>& vectorToFill) {
    if (arrayLen == 0 || byteArray == nullptr) {
        return;
    }

    size_t byteCount = 0;

    while (byteCount < arrayLen) {
        LengthAndIpPrefix newLengthAndIpPrefix;

        auto prefixLength                 = byteArray[byteCount];
        newLengthAndIpPrefix.prefixLength = prefixLength;
        byteCount++;


        if (prefixLength > 0 && prefixLength <= 8) {
            if (arrayLen - byteCount < 1) {
                vectorToFill.clear();
                return;
            }
            uint8_t octets[4]             = {byteArray[byteCount], 0, 0, 0};
            newLengthAndIpPrefix.ipPrefix = pcpp::IPv4Address(octets);
            byteCount += 1;

        } else if (prefixLength > 8 && prefixLength <= 16) {
            if (arrayLen - byteCount < 2) {
                vectorToFill.clear();
                return;
            }
            uint8_t octets[4] = {
                byteArray[byteCount], byteArray[byteCount + 1], 0, 0};
            newLengthAndIpPrefix.ipPrefix = pcpp::IPv4Address(octets);
            byteCount += 2;

        } else if (prefixLength > 16 && prefixLength <= 24) {
            if (arrayLen - byteCount < 3) {
                vectorToFill.clear();
                return;
            }
            uint8_t octets[4]             = {byteArray[byteCount],
                                 byteArray[byteCount + 1],
                                 byteArray[byteCount + 2],
                                 0};
            newLengthAndIpPrefix.ipPrefix = pcpp::IPv4Address(octets);
            byteCount += 3;

        } else if (prefixLength > 24 && prefixLength <= 32) {
            if (arrayLen - byteCount < 4) {
                vectorToFill.clear();
                return;
            }
            uint8_t octets[4]             = {byteArray[byteCount],
                                 byteArray[byteCount + 1],
                                 byteArray[byteCount + 2],
                                 byteArray[byteCount + 3]};
            newLengthAndIpPrefix.ipPrefix = pcpp::IPv4Address(octets);
            byteCount += 4;
        }

        vectorToFill.push_back(newLengthAndIpPrefix);
    }
}

std::string LengthAndIpPrefix::toString() const {
    std::string output;

    std::bitset<32> mask;
    mask.set();
    mask >>= prefixLength;
    mask.flip();


    pcpp::IPv4Address ipPrefixClean(
        htobe32(be32toh(ipPrefix.toInt()) & (uint32_t)mask.to_ulong()));

    output += "- " + ipPrefixClean.toString() + " /" +
              std::to_string(prefixLength) + "\n";

    return output;
}
