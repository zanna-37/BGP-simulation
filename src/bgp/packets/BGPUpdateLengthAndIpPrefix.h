#ifndef BGP_SIMULATION_BGP_PACKETS_BGPUPDATELENGTHANDIPPREFIX_H
#define BGP_SIMULATION_BGP_PACKETS_BGPUPDATELENGTHANDIPPREFIX_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "BGPUpdateLayer.fwd.h"
#include "IpAddress.h"
#include "../../logger/Logger.h"


class LengthAndIpPrefix {
   public:
    uint8_t           prefixLength;
    pcpp::IPv4Address ipPrefix;

    /**
     * A default c'tor that zeroes all data
     */
    LengthAndIpPrefix() : prefixLength(0), ipPrefix(pcpp::IPv4Address::Zero) {}

    /**
     * A c'tor that initializes the values of the struct
     * @param[in] prefixLength IPv4 address mask value
     * @param[in] ipPrefix IPv4 address as a string
     */
    LengthAndIpPrefix(uint8_t prefixLength, const std::string& ipPrefix)
        : prefixLength(prefixLength), ipPrefix(ipPrefix) {}

    std::string toString() const;

    /**
     * @brief Copute the length of the prfix(NetMask bits) given a IPv4 NetMask
     * 
     * @param netMask NetMask on wich to compute the length
     * @return uint8_t 
     */
    static uint8_t computeLengthIpPrefix(pcpp::IPv4Address netMask);

   private:
    /**
     * Transform the data contained in \a vector into bytes.
     * @param[in] vector The vector to extract the data from.
     * @param[out] arrayToFill The array to fill with the content of the \a
     * vector.
     * @param[in] maximumSize The maximum size to write, usually it
     * corresponds to the maximum size of the \a arrayToFill.
     * @return The amount of bytes written or 0 in case the \a maximumSize
     * cannot fit the entire content of the \a vector.
     */
    static size_t lengthAndIpPrefixDataToByteArray(
        const std::vector<LengthAndIpPrefix>& vector,
        uint8_t*                              arrayToFill,
        size_t                                maximumSize);

    /**
     * Parse the content in \a byteArray and fill \a vectorToFill.
     * @param[in] byteArray The source of bytes to parse.
     * @param[in] arrayLen The maximum length \a byteArray is parsed for.
     * @param[out] vectorToFill The output of the parsing or empty in case
     * the \a arrayLen is incorrect and doesn't allow a perfect parse of the
     * byteArray.
     */
    static void parsePrefixAndIPData(
        uint8_t*                        byteArray,
        size_t                          arrayLen,
        std::vector<LengthAndIpPrefix>& vectorToFill);

    friend class BGPUpdateLayer;
};

#endif  // BGP_SIMULATION_BGP_PACKETS_BGPUPDATELENGTHANDIPPREFIX_H
