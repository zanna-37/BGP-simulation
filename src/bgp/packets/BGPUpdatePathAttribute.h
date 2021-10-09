#ifndef BGP_SIMULATION_BGP_PACKETS_BGPUPDATEPATHATTRIBUTE_H
#define BGP_SIMULATION_BGP_PACKETS_BGPUPDATEPATHATTRIBUTE_H

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include "../../logger/Logger.h"


/**
 * @warning This datastructure does not check the validity of the data.
 * Ensure the length is congruent to the data before using its variables and
 * methods, otherwise, a buffer overflow error is ensured.
 */
class PathAttribute {
   public:
    enum AttributeTypeFlags_uint8_t : uint8_t {
        OPTIONAL   = 1 << 7,  // binary 1000'0000
        TRANSITIVE = 1 << 6,  // binary 0100'0000
        PARTIAL    = 1 << 5,  // binary 0010'0000
        EXTENDED   = 1 << 4,  // binary 0001'0000
    };

    enum AttributeTypeCode_uint8_t : uint8_t {
        ORIGIN           = 1,
        AS_PATH          = 2,
        NEXT_HOP         = 3,
        MULTI_EXIT_DISC  = 4,
        LOCAL_PREF       = 5,
        ATOMIC_AGGREGATE = 6,
        AGGREGATOR       = 7
    };

    uint8_t                   attributeTypeFlags = 0;
    AttributeTypeCode_uint8_t attributeTypeCode{};

   private:
    uint8_t attributeLength_normal{};
    uint8_t attributeLength_higherBits{};

    /**
     * Attribute value(s). There is no specification on the content of this
     * variable because it is dependent on the attribute type. The only
     * requirement is that the data fields saved inside are represented
     * (singularly) in big endian byte order.
     */
    uint8_t* attributeData_be = nullptr;

   public:
    static bool isFlagSet(uint8_t                    flagsToTest,
                          AttributeTypeFlags_uint8_t flagValue) {
        return (flagsToTest & flagValue) == flagValue;
    };

    [[nodiscard]] bool isFlagSet(AttributeTypeFlags_uint8_t flagValue) const {
        return isFlagSet(this->attributeTypeFlags, flagValue);
    };

    void setFlags(AttributeTypeFlags_uint8_t flagType, bool value) {
        /**
         * @example
         * \verbatim
         * with:
         *     flagType   = 00001000;
         *     value = true;    |
         * result:              v
         *     value_bits = 00001000
         *
         * with:
         *     flagType   = 00001000;
         *     value = false;   |
         * result:              v
         *     value_bits = 00000000
         * \endverbatim
         */
        AttributeTypeFlags_uint8_t value_bits =
            value ? flagType : (AttributeTypeFlags_uint8_t)0;

        auto mask = (uint8_t)~flagType;

        /**
         * \verbatim
         * with:
         *     value_bits               = 00001000;
         *     flagType                 = 00001000;
         *     mask                     = 11110111;
         * result:
         * [in]        oldAttributeTypeFlags 01000000
         * [operation]              AND mask 11110111   --> make is made from
         *                                    |  |          ~value_bits
         * [intermediate result]             01000000
         *                                    |  |
         *                                    |  +-------> zeroed
         *                                    +--|-------> preserved
         *                                    |  |
         * [operation]         OR value_bits 00001000
         *                                    |  |
         * [out]       newAttributeTypeFlags 01001000
         *
         * with:
         *     value_bits               = 00001000;
         *     flagType                 = 00001000;
         *     mask                     = 11110111;
         * result:
         * [in]        oldAttributeTypeFlags 01001000
         * [operation]              AND mask 11110111 ---> make is made from
         *                                    |  |         ~value_bits
         * [intermediate result]             01000000
         *                                    |  |
         *                                    |  +-------> zeroed
         *                                    +--|-------> preserved
         *                                    |  |
         * [operation]         OR value_bits 00001000
         *                                    |  |
         * [out]       newAttributeTypeFlags 01001000
         * \endverbatim
         */
        this->attributeTypeFlags =
            (uint8_t)((this->attributeTypeFlags & mask) | value_bits);
    }

    /**
     * Get the attribute length.
     * @warning The result is always expressed in \a uint16_t even if the \a
     * EXTENDED bit is not set.
     * @return The attribute length is host byte order.
     */
    uint16_t getAttributeLength_h() const {
        if (isFlagSet(attributeTypeFlags, EXTENDED)) {
            return ((uint16_t)attributeLength_higherBits << 8) |
                   attributeLength_normal;
        } else {
            return attributeLength_normal;
        }
    }

    /**
     * Get a pointer to the attribute data.
     * @return Get a pointer to the attribute data in big endian byte order.
     */
    uint8_t* getAttributeValue_be() const { return attributeData_be; }

    /**
     * Set the length of the attribute and its value taking care of setting the
     * \a EXTENDED bit flag when the size is larger that a byte.
     * @param data_be The value to store in BigEndian format.
     * @param attributeSize The size of \a data_be
     */
    void setAttributeLengthAndValue(uint8_t* data_be, uint16_t attributeSize) {
        bool extended = attributeSize > UINT8_MAX;
        if (extended) {
            setFlags(EXTENDED, true);
        }

        this->attributeLength_normal = (uint8_t)attributeSize;
        if (extended) {
            this->attributeLength_higherBits = (attributeSize >> 8);
        }

        this->attributeData_be = data_be;
    }

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
    static size_t pathAttributesToByteArray(
        const std::vector<PathAttribute>& vector,
        uint8_t*                          arrayToFill,
        size_t                            maximumSize);

    /**
     * Parse the content in \a byteArray and fill \a vectorToFill.
     * @param[in] byteArray The source of bytes to parse.
     * @param[in] arrayLen The maximum length \a byteArray is parsed for.
     * @param[out] vectorToFill The output of the parsing or empty in case
     * the \a arrayLen is incorrect and doesn't allow a perfect parse of the
     * byteArray.
     */
    static void parsePathAttributes(uint8_t*                    byteArray,
                                    size_t                      arrayLen,
                                    std::vector<PathAttribute>& vectorToFill);

    /**
     * @brief Build the segment for the AS_PATH data attribute
     *
     * @param[in] asType Segment type of the AS_PATH
     * @param[in] asPathLen Number of ASs in the path
     * @param[in] asPath Vector with the AS's numbers
     * @param[out] asData_be Attribute data for the AS_PATH attribute construction
     */
    static void buildAsPathAttributeData_be(uint8_t asType,
                                            uint8_t asPathLen,
                                            const std::vector<uint16_t>& asPath,
                                            std::vector<uint8_t>& asData_be);

    std::string toString() const;
};

#endif  // BGP_SIMULATION_BGP_PACKETS_BGPUPDATEPATHATTRIBUTE_H
