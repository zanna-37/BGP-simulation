#ifndef BGPSIMULATION_ENTITIES_LINK_H
#define BGPSIMULATION_ENTITIES_LINK_H

#include <Packet.h>
#include <assert.h>
#include <time.h>

#include "NetworkCard.h"

// forward declarations
#include "NetworkCard.fwd.h"

using namespace std;

enum Connection_status {
    /**
     * The link is up and healthy. All the packets pass through it successfully.
     */
    ACTIVE,
    /**
     * The link is broken. No packet can pass though it.
     */
    FAILED
};

/**
 * This class abstracts the concept of a medium (e.g. a cable) that connects two
 * devices.
 */
class Link {
   public:
    Connection_status connection_status;

    Link(Connection_status connection_status)
        : connection_status(connection_status) {}

    /**
     * Get the networkCard of the opposite endpoint in the link.
     *
     * @param networkCard The networkCard whose peer address will be returned.
     * @return The networkCard of the opposite endpoint in the link or \a
     * nullptr if there is not such endpoint connected or if the specified
     * networkCard is not connected to the link.
     */
    NetworkCard *getPeerNetworkCardOrNull(const NetworkCard *networkCard) const;

    /**
     * Send a packet through the physical link. The packet is serialized, so it
     * is a stream of bytes
     * @warning it should be called by the Network card
     * @param packet the packet to send
     * @param destination the source network card
     */
    void sendPacket(const pcpp::Packet *packet, NetworkCard *destination) const;

    /**
     * The physical link receive a packet (instantaneously, no network delay).
     * @warning This should be called by the Link.
     * @param receivedDataStream The pair indicating the length and the array of
     * bytes.
     * @param destination The networkCard where the packet arrived.
     */
    static void receivePacket(
        pair<const uint8_t *, const int> receivedDataStream,
        NetworkCard *                    destination);
    /**
     * Get the read-only array-of-bytes representation of the parsed packet.
     * @param packet The packet to be serialized.
     * @return A pair with the data and the data length.
     */
    static pair<const uint8_t *, const int> serialize(
        const pcpp::Packet *packet);

    /**
     * Put the stream of bytes in a parsed packet.
     *
     * The newly created packet will be in charge of deallocating the rawData on
     * destruction.
     *
     * @param rawdata The raw data to wrap.
     * @param rawDataLen The length of the stream.
     * @return A parsed packet.
     */
    static unique_ptr<pcpp::Packet> deserialize(uint8_t *rawData,
                                                int      rawDataLen);

   private:
    /**
     * Convenient pair that stores the references to the connected devices. No
     * order is assumed.
     *
     * @warning This must always reflect the state of the \a NetworkCards. It is
     * kept in sync by the methods \a Link::connect(...) and \a
     * Link::disconnect(...) which are in turn called from the \a NetworkCard
     * class. For this reason this pair should be used as a convenient reference
     * but the ground truth is always the state inside the \a NetworkCards.
     */
    pair<NetworkCard *, NetworkCard *> device_source_networkCards;

    /**
     * Disassociate the specified networkCard.
     * This reflects the changes performed in \a NeworkCard::disconnect(...). In
     * particular, when a network card disassociate from a link, the link will
     * in turn remove its reference of the (now removed) network card.
     *
     * @warning This is an internal method and should only be called from the \a
     * NetworkCard class. Use \a NeworkCard::disconnect(...) instead.
     * @param networkCard The networkcard that has been disassociated.
     */
    void disconnect(NetworkCard *networkCard);

    /**
     * Associate the specified networkCard.
     * This reflects the changes performed in \a NeworkCard::connect(...). In
     * particular, when a network card associate with a link, the link stores an
     * internal reference of the newly attached network card.
     *
     * @warning This is an internal method and should only be called from the \a
     * NetworkCard class.  Use \a NeworkCard::connect(...) instead.
     * @param networkCard The networkcard that has been associated.
     */
    void connect(NetworkCard *networkCard);

    friend class NetworkCard;
};


#endif  // BGPSIMULATION_ENTITIES_LINK_H
